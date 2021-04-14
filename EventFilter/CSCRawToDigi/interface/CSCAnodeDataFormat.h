#ifndef EventFilter_CSCRawToDigi_CSCAnodeDataFormat_h
#define EventFilter_CSCRawToDigi_CSCAnodeDataFormat_h

#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include <vector>

class CSCAnodeDataFormat {
public:
  virtual ~CSCAnodeDataFormat() {}
  virtual unsigned short* data() = 0;
  /// the amount of the input binary buffer read, in 16-bit words
  virtual unsigned short int sizeInWords() const = 0;

  /// input layer is from 1 to 6
  virtual std::vector<CSCWireDigi> wireDigis(int layer) const = 0;

  virtual void add(const CSCWireDigi& wireDigi, int layer) = 0;
};

#endif
