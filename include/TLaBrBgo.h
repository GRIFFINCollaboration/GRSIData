#ifndef TLABRBGO_H
#define TLABRBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include "Globals.h"
#include "TBgo.h"

class TLaBrBgo : public TBgo {
public:
   TLaBrBgo();
   TLaBrBgo(const TLaBrBgo&);
   virtual ~TLaBrBgo();

   TLaBrBgo& operator=(const TLaBrBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TLaBrBgo, 1) // LaBrBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
