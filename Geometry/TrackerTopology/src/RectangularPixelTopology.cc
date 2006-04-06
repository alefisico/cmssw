#include "Geometry/TrackerTopology/interface/RectangularPixelTopology.h"

  /**
   * Topology for rectangular pixel detector with BIG pixels.
   */
// Modified for the large pixles.
// Danek Kotlinski & Michele Pioppi, 3/06.
// See documentation in the include file.

// PixelTopology interface. 
// Transform LocalPoint in cm to measurement in pitch units.
std::pair<float,float> RectangularPixelTopology::pixel( 
	  const LocalPoint& p) const {
  // check limits	
  float py = p.y();
  float px = p.x();
  
  if(DEBUG) {
    // This will catch points which are outside the active sensor area.
    // In the digitizer during the early induce_signal phase non valid
    // location are passed here. They are cleaned later.
    if( py<m_yoffset ) { // m_yoffset is negative 
      cout<<" wrong lp y "<<setprecision(7)<<py<<" "<<m_yoffset<<endl;
      py = m_yoffset + EPSCM; // make sure it is in, add an EPS in cm
    }
    if( py>-m_yoffset ) {
      cout<<" wrong lp y "<<setprecision(7)<<py<<" "<<-m_yoffset<<endl;
      py = -m_yoffset - EPSCM;
    }
    if( px<m_xoffset ) { // m_xoffset is negative 
      cout<<" wrong lp x "<<setprecision(7)<<px<<" "<<m_xoffset<<endl;
      px = m_xoffset + EPSCM;
    }
    if( px>-m_xoffset ) {
      cout<<" wrong lp x "<<setprecision(7)<<px<<" "<<-m_xoffset<<endl;
      px = -m_xoffset - EPSCM;
    }
  } // end DEBUG

  float newybin=(py - m_yoffset)/m_pitchy;
  int iybin = int(newybin);
  float fractionY = newybin - iybin;
  //if(fractionY<0.) cout<<" fractiony "<<fractionY<<" "<<newybin<<endl;
  
  // Normalize it all to 1 ROC
  int iybin0 = (iybin%54); // 0-53
  int numROC = iybin/54;  // 0-7
  
  if (iybin0>53) {
    if(DEBUG) {
      cout<<" very bad, newbiny "<<iybin0<<setprecision(10)<<endl;
      cout<<py<<" "<<m_yoffset<<" "<<m_pitchy<<" "
	  <<newybin<<" "<<iybin<<" "<<fractionY<<" "<<iybin0<<" "
	  <<numROC<<endl;
    }
  } else if (iybin0==53) {   // inside big pixel
    iybin0=51;
    fractionY = (fractionY+1.)/2.;
  } else if (iybin0==52) {   // inside big pixel
    iybin0=51;
    fractionY = fractionY/2.;
  } else if (iybin0>1) {   // inside normal pixel
    iybin0=iybin0-1;
  } else if (iybin0==1) {   // inside big pixel
    iybin0=0;
    fractionY = (fractionY+1.)/2.;
  } else if (iybin0==0) {   // inside big pixel
    iybin0=0;
    fractionY = fractionY/2.;
  } else {
    if(DEBUG) {
      cout<<" very bad, newbiny "<<newybin<<setprecision(10)<<endl;
      cout<<py<<" "<<m_yoffset<<" "<<m_pitchy<<" "
	  <<newybin<<" "<<iybin<<" "<<fractionY<<" "
	  <<iybin0<<" "<<numROC<<endl;
    }
  }
  float mpY = float(numROC*52. + iybin0) + fractionY;
  if(DEBUG && (mpY<0. || mpY>=416.)) {
    cout<<" bad pix y "<<setprecision(10)<<mpY<<endl;
    cout<<py<<" "<<m_yoffset<<" "<<m_pitchy<<" "
	<<newybin<<" "<<iybin<<" "<<fractionY<<" "
	<<iybin0<<" "<<numROC<<endl;
  }
  
  // In X
  float newxbin=(px - m_xoffset) / m_pitchx; 
  int ixbin = int(newxbin);
  float fractionX = newxbin - ixbin;
  // if(fractionX<0.) {
  //   cout<<" fractionx "<<fractionX<<" "<<newxbin<<" "<<ixbin<<" ";
  //   cout<<px<<" "<<m_xoffset<<" "<<m_pitchx<<" "
  // 	  <<newxbin<<" "<<ixbin<<" "<<fractionX<<endl;
  // }

  if (ixbin>161) {
    if(DEBUG) {
      cout<<" very bad, newbinx "<<ixbin<<setprecision(10)<<endl;
      cout<<px<<" "<<m_xoffset<<" "<<m_pitchx<<" "
	  <<newxbin<<" "<<ixbin<<" "<<fractionX<<endl;
    }
  } else if (ixbin>82) {   // inside normal pixel, ROC 1 
    ixbin=ixbin-2;
  } else if (ixbin==82) {   // inside bin pixel 
    ixbin=80;
    fractionX = (fractionX+1.)/2.;
  } else if (ixbin==81) {   // inside big pixel
    ixbin=80;
    fractionX = fractionX/2.;
  } else if (ixbin==80) {   // inside bin pixel, ROC 0 
    ixbin=79;
    fractionX = (fractionX+1.)/2.;
  } else if (ixbin==79) {   // inside big pixel
    ixbin=79;
    fractionX = fractionX/2.;
  } else if (ixbin<0) {   // outside range
    if(DEBUG) {
      cout<<" very bad, newbinx "<<ixbin<<setprecision(10)<<endl;
      cout<<px<<" "<<m_xoffset<<" "<<m_pitchx<<" "
	  <<newxbin<<" "<<ixbin<<" "<<fractionX<<endl;
    }
  }
  
  float mpX = float(ixbin) + fractionX;
  
  if(DEBUG && (mpX<0. || mpX>=160.) ) {
    cout<<" bad pix x "<<mpX<<" "<<setprecision(10)<<endl;
    cout<<px<<" "<<m_xoffset<<" "<<m_pitchx<<" "
	<<newxbin<<" "<<ixbin<<" "<<fractionX<<endl;
  }
  
  return std::pair<float,float>(mpX,mpY);
}
//
// Topology interface, go from Masurement to Local corrdinates
// pixel coordinates (mp) -> cm (LocalPoint)
LocalPoint RectangularPixelTopology::localPosition( 
        const MeasurementPoint& mp) const {

  float mpy = mp.y(); // measurements 
  float mpx = mp.x();

  // check limits
  if(DEBUG) {
    if( mpy<0.) { //  
      cout<<" wrong mp y, fix "<<setprecision(7)<<mpy<<" "
	  <<0<<endl;
      mpy = 0.;
    }
    if( mpy>=m_ncols) {
      cout<<" wrong mp y, fix "<<setprecision(7)<<mpy<<" "
	  <<m_ncols<<endl;
      mpy = float(m_ncols) - EPS; // EPS is a small number
    }
    if( mpx<0.) { //  
      cout<<" wrong mp x, fix "<<setprecision(7)<<mpx<<" "
	  <<0<<endl;
      mpx = 0.;
    }
    if( mpx>=m_nrows) {
      cout<<" wrong mp x, fix "<<setprecision(7)<<mpx<<" "
	  <<m_nrows<<endl;
      mpx = float(m_nrows) - EPS; // EPS is a small number
    }
  } // if DEBUG
  
  // Start with Y
  int binoffy = int(mpy);             // truncate to int
  float fractionY = mpy - binoffy; // find the fraction 
  float local_pitchy = m_pitchy;      // defaultpitch
  //if(fractionY<0.) cout<<" fractiony m "<<fractionY<<" "<<mpy<<endl;

  if (binoffy>415) {   // too large
    if(DEBUG) { 
      cout<<" very bad, biny "<<binoffy<<setprecision(10)<<endl;
      cout<<mpy<<" "<<binoffy<<" "
	  <<fractionY<<" "<<local_pitchy<<" "<<m_yoffset<<endl;
    }
  } else if (binoffy==415) {    // ROC 7, last big pixel
    binoffy=binoffy+15;
    local_pitchy = 2 * m_pitchy;
  } else if (binoffy>364) {     // ROC 7
    binoffy=binoffy+15;
  } else if (binoffy==364) {    // ROC 7
    binoffy=binoffy+14;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==363) {      // ROC 6
    binoffy=binoffy+13;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>312) {       // ROC 6
    binoffy=binoffy+13;
  } else if (binoffy==312) {      // ROC 6
    binoffy=binoffy+12;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==311) {      // ROC 5
    binoffy=binoffy+11;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>260) {       // ROC 5
    binoffy=binoffy+11;
  } else if (binoffy==260) {      // ROC 5
    binoffy=binoffy+10;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==259) {      // ROC 4
    binoffy=binoffy+9;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>208) {       // ROC 4
    binoffy=binoffy+9;
  } else if (binoffy==208) {      // ROC 4
    binoffy=binoffy+8;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==207) {      // ROC 3
    binoffy=binoffy+7;
    local_pitchy = 2 * m_pitchy;    
    } else if (binoffy>156) {       // ROC 3
    binoffy=binoffy+7;
  } else if (binoffy==156) {      // ROC 3
    binoffy=binoffy+6;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==155) {      // ROC 2
    binoffy=binoffy+5;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>104) {       // ROC 2
    binoffy=binoffy+5;
  } else if (binoffy==104) {      // ROC 2
    binoffy=binoffy+4;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==103) {      // ROC 1
    binoffy=binoffy+3;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>52) {       // ROC 1
    binoffy=binoffy+3;
  } else if (binoffy==52) {      // ROC 1
    binoffy=binoffy+2;
    local_pitchy = 2 * m_pitchy;
    
  } else if (binoffy==51) {      // ROC 0
    binoffy=binoffy+1;
    local_pitchy = 2 * m_pitchy;    
  } else if (binoffy>0) {        // ROC 0
    binoffy=binoffy+1;
  } else if (binoffy==0) {       // ROC 0
    binoffy=binoffy+0;
    local_pitchy = 2 * m_pitchy;
  } else { // too small
    if(DEBUG) { 
      cout<<" very bad, biny "<<binoffy<<setprecision(10)<<endl;
      cout<<mpy<<" "<<binoffy<<" "
	  <<fractionY<<" "<<local_pitchy<<" "<<m_yoffset<<endl;
    }
  }
  
  // The final position in local coordinates 
  float lpY = float(binoffy*m_pitchy) + fractionY*local_pitchy + 
    m_yoffset;
  if(DEBUG && (lpY<m_yoffset || lpY>(-m_yoffset)) ) {
    cout<<" bad lp y "<<lpY<<setprecision(10)<<endl; 
    cout<<mpy<<" "<<binoffy<<" "
	<<fractionY<<" "<<local_pitchy<<" "<<m_yoffset<<endl;
  }
  
  // Do the X
  int binoffx=int(mpx);             // truncate to int
  float fractionX = mpx - binoffx; // find the fraction 
  float local_pitchx = m_pitchx;      // defaultpitch
  //if(fractionX<0.) cout<<" fractionx m "<<fractionX<<" "<<mpx<<endl;
  
  if (binoffx>159) {   // too large
    if(DEBUG) { 
      cout<<" very bad, binx "<<binoffx<<setprecision(10)<<endl;
      cout<<mpx<<" "<<binoffx<<" "
	  <<fractionX<<" "<<local_pitchx<<" "<<m_xoffset<<endl;
    }
  } else if (binoffx>80) {     // ROC 1
    binoffx=binoffx+2;
  } else if (binoffx==80) {    // ROC 1
    binoffx=binoffx+1;
    local_pitchx = 2 * m_pitchx;
    
  } else if (binoffx==79) {      // ROC 0
    binoffx=binoffx+0;
    local_pitchx = 2 * m_pitchx;    
  } else if (binoffx>=0) {       // ROC 0
    binoffx=binoffx+0;
    
  } else { // too small
    if(DEBUG) { 
      cout<<" very bad, binx "<<binoffx<<setprecision(10)<<endl;
      cout<<mpx<<" "<<binoffx<<" "
	  <<fractionX<<" "<<local_pitchx<<" "<<m_xoffset<<endl;
    }
  }
  
  // The final position in local coordinates 
  float lpX = float(binoffx*m_pitchx) + fractionX*local_pitchx + 
    m_xoffset;
  
  if(DEBUG && (lpX<m_xoffset || lpX>(-m_xoffset)) ) {
    cout<<" bad lp x "<<lpX<<setprecision(10)<<endl; 
    cout<<mpx<<" "<<binoffx<<" "
	<<fractionX<<" "<<local_pitchx<<" "<<m_xoffset<<endl;
  }
  
  // Return it
  return LocalPoint( lpX, lpY);
}
///////////////////////////////////////////////////////////////////
// Get hit errors in LocalPoint coordinates (cm)
LocalError RectangularPixelTopology::localError( 
				      const MeasurementPoint& mp,
				      const MeasurementError& me) const {
  float pitchy=m_pitchy;
  int binoffy=int(mp.y());
  if( isItBigPixelInY(binoffy) )pitchy = 2.*m_pitchy;

  float pitchx=m_pitchx;
  int binoffx=int(mp.x());
  if( isItBigPixelInX(binoffx) )pitchx = 2.*m_pitchx;

  return LocalError( me.uu()*float(pitchx*pitchx), 0,
		     me.vv()*float(pitchy*pitchy));
}
/////////////////////////////////////////////////////////////////////
// Get errors in pixel pitch units.
MeasurementError RectangularPixelTopology::measurementError( 
		 const LocalPoint& lp,
		 const LocalError& le) const {

  float pitchy=m_pitchy;
  int iybin = int( (lp.y() - m_yoffset)/m_pitchy );   //get bin for equal picth 
  int iybin0 = iybin%54;  //This is just to avoid many ifs by using the periodicy
  //quasi bins 0,1,52,53 fall into larger pixels  
  if(iybin0==0 || iybin0==1 || iybin0==52 || iybin0==53 )
    pitchy = 2. * m_pitchy;

  float pitchx=m_pitchx;
  int ixbin = int( (lp.x() - m_xoffset)/m_pitchx );   //get bin for equal pitch
  //quasi bins 79,80,81,82 fall into the 2 larger pixels  
  if(ixbin>=79 && ixbin<=82) pitchx = 2. * m_pitchx;  

  return MeasurementError( le.xx()/float(pitchx*pitchx), 0,
			   le.yy()/float(pitchy*pitchy));
}
