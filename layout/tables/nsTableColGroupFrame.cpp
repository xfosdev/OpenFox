/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
#include "nsTableColGroupFrame.h"
#include "nsTableColFrame.h"
#include "nsIReflowCommand.h"
#include "nsIStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIPresContext.h"
#include "nsIPtr.h"
#include "nsIContentDelegate.h"
#include "nsHTMLAtoms.h"

NS_DEF_PTR(nsIContent);
NS_DEF_PTR(nsIStyleContext);

static PRBool gsDebug = PR_FALSE;


nsTableColGroupFrame::nsTableColGroupFrame(nsIContent* aContent,
                     nsIFrame*   aParentFrame)
  : nsContainerFrame(aContent, aParentFrame)
{
  mColCount=0;
}

nsTableColGroupFrame::~nsTableColGroupFrame()
{
}

NS_METHOD nsTableColGroupFrame::Paint(nsIPresContext& aPresContext,
                                      nsIRenderingContext& aRenderingContext,
                                      const nsRect&        aDirtyRect)
{
  if (gsDebug==PR_TRUE) printf("nsTableColGroupFrame::Paint\n");
  PaintChildren(aPresContext, aRenderingContext, aDirtyRect);
  return NS_OK;
}

// TODO:  content changed notifications
NS_METHOD nsTableColGroupFrame::Reflow(nsIPresContext*      aPresContext,
                                       nsReflowMetrics&     aDesiredSize,
                                       const nsReflowState& aReflowState,
                                       nsReflowStatus&      aStatus)
{
  NS_ASSERTION(nsnull!=aPresContext, "bad arg");
  NS_ASSERTION(nsnull!=mContent, "bad state -- null content for frame");

  if (nsnull == mFirstChild) 
  { // if we don't have any children, create them
    nsIFrame* kidFrame = nsnull;
    nsIFrame* prevKidFrame;
   
    LastChild(prevKidFrame);  // XXX remember this...
    PRInt32 kidIndex = 0;
    for (PRInt32 colIndex = 0; ;colIndex++) // colIndex is used to set the column frames' index field
    {
      nsIContentPtr kid = mContent->ChildAt(kidIndex);   // kid: REFCNT++
      if (kid.IsNull()) {
        break;
      }

      // Resolve style
      nsIStyleContextPtr kidSC =
        aPresContext->ResolveStyleContextFor(kid, this, PR_TRUE);
      const nsStyleSpacing* kidSpacing = (const nsStyleSpacing*)
        kidSC->GetStyleData(eStyleStruct_Spacing);

      // Create a child frame
      nsIContentDelegate* kidDel = nsnull;
      kidDel = kid->GetDelegate(aPresContext);
      nsresult rv = kidDel->CreateFrame(aPresContext, kid, this, kidSC,
                                        kidFrame);
      NS_RELEASE(kidDel);

      // give the child frame a chance to reflow, even though we know it'll have 0 size
      nsReflowMetrics kidSize(nsnull);
      nsReflowState   kidReflowState(kidFrame, aReflowState, nsSize(0,0), eReflowReason_Initial);
      kidFrame->WillReflow(*aPresContext);
      nsReflowStatus status = ReflowChild(kidFrame,aPresContext, kidSize,
                                          kidReflowState);
      // note that DidReflow is called as the result of some ancestor firing off a DidReflow above me
      kidFrame->SetRect(nsRect(0,0,0,0));

      // set nsColFrame-specific information
      ((nsTableColFrame *)kidFrame)->SetColumnIndex(colIndex+mStartColIndex);

      // Link child frame into the list of children
      if (nsnull != prevKidFrame) {
        prevKidFrame->SetNextSibling(kidFrame);
      } else {
        mFirstChild = kidFrame;  // our first child
        SetFirstContentOffset(kidIndex);
      }
      prevKidFrame = kidFrame;
      mChildCount++;
      kidIndex++;
    }
    // now that I have all my COL children, adjust their style
    SetStyleContextForFirstPass(aPresContext);
  }
  aDesiredSize.width=0;
  aDesiredSize.height=0;
  if (nsnull!=aDesiredSize.maxElementSize)
  {
    aDesiredSize.maxElementSize->width=0;
    aDesiredSize.maxElementSize->height=0;
  }
  aStatus = NS_FRAME_COMPLETE;
  return NS_OK;
}

// Subclass hook for style post processing
NS_METHOD nsTableColGroupFrame::SetStyleContextForFirstPass(nsIPresContext* aPresContext)
{
  // get the table frame
  nsIFrame* tableFrame=nsnull;
  GetGeometricParent(tableFrame);
  tableFrame->GetGeometricParent(tableFrame); // get the outer frame
  
  // get the style for the table frame
  nsIStyleContextPtr tableSC;
  tableFrame->GetStyleContext(aPresContext, tableSC.AssignRef());
  nsStyleTable *tableStyle = (nsStyleTable*)tableSC->GetStyleData(eStyleStruct_Table);

  // if COLS is set, then map it into the COL frames
  if (NS_STYLE_TABLE_COLS_NONE != tableStyle->mCols)
  {
    // set numCols to the number of columns effected by the COLS attribute
    PRInt32 numCols=0;
    if (NS_STYLE_TABLE_COLS_ALL == tableStyle->mCols)
      ChildCount(numCols);
    else 
      numCols = tableStyle->mCols;

    // for every column effected, set its width style
    PRInt32 colIndex=0;
    nsIFrame *colFrame=nsnull;
    nsIStyleContextPtr colStyleContext;
    for (; colIndex<numCols; colIndex++)
    {
      ChildAt(colIndex, colFrame);
      if (nsnull==colFrame)
        break;  // the attribute value specified was greater than the actual number of columns
      nsStylePosition * colPosition=nsnull;
      colFrame->GetStyleContext(aPresContext, colStyleContext.AssignRef());
      colPosition = (nsStylePosition*)colStyleContext->GetMutableStyleData(eStyleStruct_Position);
      nsStyleCoord width (1, eStyleUnit_Proportional);
      colPosition->mWidth = width;
      colStyleContext->RecalcAutomaticData(aPresContext);
    }
    // if there are more columns, there width is set to "minimum"
    PRInt32 numChildFrames;
    ChildCount(numChildFrames);
    for (; colIndex<numChildFrames; colIndex++)
    {
      ChildAt(colIndex, colFrame);
      NS_ASSERTION(nsnull!=colFrame, "bad column frame");
      nsStylePosition * colPosition=nsnull;
      colFrame->GetStyleContext(aPresContext, colStyleContext.AssignRef());
      colPosition = (nsStylePosition*)colStyleContext->GetMutableStyleData(eStyleStruct_Position);
      colPosition->mWidth.SetCoordValue(0);
      colStyleContext->RecalcAutomaticData(aPresContext);
    }

    mStyleContext->RecalcAutomaticData(aPresContext);
  }
  return NS_OK;
}


/** returns the number of columns represented by this group.
  * if there are col children, count them (taking into account the span of each)
  * else, check my own span attribute.
  */
int nsTableColGroupFrame::GetColumnCount ()
{
  if (0 == mColCount)
  {
    int count;
    ChildCount (count);
    if (0 < count)
    {
      nsIFrame * child = nsnull;
      ChildAt(0, child);
      NS_ASSERTION(nsnull!=child, "bad child");
      while (nsnull!=child)
      {
        nsTableColFrame *col = (nsTableColFrame *)child;
        col->SetColumnIndex (mStartColIndex + mColCount);
        mColCount += col->GetRepeat ();
        child->GetNextSibling(child);
      }
    }
    else
    {
      const nsStyleTable *tableStyle;
      GetStyleData(eStyleStruct_Table, (nsStyleStruct *&)tableStyle);
      mColCount = tableStyle->mSpan;
    }
  }
  return mColCount;
}

/* ----- static methods ----- */

nsresult nsTableColGroupFrame::NewFrame(nsIFrame** aInstancePtrResult,
                                        nsIContent* aContent,
                                        nsIFrame*   aParent)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  nsIFrame* it = new nsTableColGroupFrame(aContent, aParent);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  *aInstancePtrResult = it;
  return NS_OK;
}


