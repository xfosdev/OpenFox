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

#include "nsFontMetricsUnix.h"
#include "nsDeviceContextUnix.h"

static NS_DEFINE_IID(kIFontMetricsIID, NS_IFONT_METRICS_IID);

nsFontMetricsUnix :: nsFontMetricsUnix()
{
  NS_INIT_REFCNT();
  mFont = nsnull;
}
  
nsFontMetricsUnix :: ~nsFontMetricsUnix()
{
  if (nsnull != mFont)
  {
    delete mFont;
    mFont = nsnull;
  }
}

NS_IMPL_ISUPPORTS(nsFontMetricsUnix, kIFontMetricsIID)

nsresult nsFontMetricsUnix :: Init(const nsFont& aFont, nsIDeviceContext* aCX)
{
  mFont = new nsFont(aFont);
  mContext = aCX ;

  RealizeFont();

  return NS_OK;
}

void nsFontMetricsUnix::RealizeFont()
{
  //mFontHandle = ::CreateFontIndirect(&logFont);

  //mHeight = nscoord(metrics.tmHeight * p2t);
  //mAscent = nscoord(metrics.tmAscent * p2t);
  //mDescent = nscoord(metrics.tmDescent * p2t);
  //mLeading = nscoord(metrics.tmInternalLeading * p2t);
  //mMaxAscent = nscoord(metrics.tmAscent * p2t);
  //mMaxDescent = nscoord(metrics.tmDescent * p2t);
  //mMaxAdvance = nscoord(metrics.tmMaxCharWidth * p2t);


  nsDrawingSurfaceUnix * aRenderingSurface = (nsDrawingSurfaceUnix *) ((nsDeviceContextUnix *)mContext)->GetDrawingSurface();

  mFontHandle = ::XLoadFont(aRenderingSurface->display, "fixed");

  XFontStruct * fs = ::XQueryFont(aRenderingSurface->display, mFontHandle);

  mAscent = fs->ascent ;
  mDescent = fs->descent ;

  ::XSetFont(aRenderingSurface->display, aRenderingSurface->gc, mFontHandle);


}

nscoord nsFontMetricsUnix :: GetWidth(char ch)
{
  return mCharWidths[PRUint8(ch)];
}

nscoord nsFontMetricsUnix :: GetWidth(PRUnichar ch)
{
  if (ch < 256) {
    return mCharWidths[ch];
  }
  return 0;/* XXX */
}

nscoord nsFontMetricsUnix :: GetWidth(const nsString& aString)
{
  return GetWidth(aString.GetUnicode(), aString.Length());
}

nscoord nsFontMetricsUnix :: GetWidth(const char *aString)
{
  // XXX use native text measurement routine
  nscoord sum = 0;
  PRUint8 ch;
  while ((ch = PRUint8(*aString++)) != 0) {
    sum += mCharWidths[ch];
  }
  return sum;
}

nscoord nsFontMetricsUnix :: GetWidth(const PRUnichar *aString, PRUint32 aLength)
{
  // XXX use native text measurement routine
  nscoord sum = 0;
  while (aLength != 0) {
    PRUnichar ch = *aString++;
    if (ch < 256) {
      sum += mCharWidths[ch];
    } else {
      // XXX not yet
    }
    --aLength;
  }
  return sum;
}

nscoord nsFontMetricsUnix :: GetHeight()
{
  return mHeight;
}

nscoord nsFontMetricsUnix :: GetLeading()
{
  return mLeading;
}

nscoord nsFontMetricsUnix :: GetMaxAscent()
{
  return mMaxAscent;
}

nscoord nsFontMetricsUnix :: GetMaxDescent()
{
  return mMaxDescent;
}

nscoord nsFontMetricsUnix :: GetMaxAdvance()
{
  return mMaxAdvance;
}

const nscoord * nsFontMetricsUnix :: GetWidths()
{
  return mCharWidths;
}

const nsFont& nsFontMetricsUnix :: GetFont()
{
  return *mFont;
}

nsFontHandle nsFontMetricsUnix :: GetFontHandle()
{
  return ((nsFontHandle)mFontHandle);
}


// XXX this function is a hack; the only logical font names we should
// support are the one used by css.
const char* nsFontMetricsUnix::MapFamilyToFont(const nsString& aLogicalFontName)
{
  if (aLogicalFontName.EqualsIgnoreCase("Times Roman")) {
    return "Times New Roman";
  }
  if (aLogicalFontName.EqualsIgnoreCase("Times")) {
    return "Times New Roman";
  }
  if (aLogicalFontName.EqualsIgnoreCase("Unicode")) {
    return "Bitstream Cyberbit";
  }
  if (aLogicalFontName.EqualsIgnoreCase("Courier")) {
    return "Courier New";
  }

  // the CSS generic names
  if (aLogicalFontName.EqualsIgnoreCase("serif")) {
    return "Times New Roman";
  }
  if (aLogicalFontName.EqualsIgnoreCase("sans-serif")) {
    return "Arial";
  }
  if (aLogicalFontName.EqualsIgnoreCase("cursive")) {
//    return "XXX";
  }
  if (aLogicalFontName.EqualsIgnoreCase("fantasy")) {
//    return "XXX";
  }
  if (aLogicalFontName.EqualsIgnoreCase("monospace")) {
    return "Courier New";
  }
  return "Arial";/* XXX for now */
}


