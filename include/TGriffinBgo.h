#ifndef TGRIFFINBGO_H
#define TGRIFFINBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include "Globals.h"
#include "TBgo.h"

class TGriffinBgo : public TBgo {
public:
   TGriffinBgo();
   TGriffinBgo(const TGriffinBgo&);
   virtual ~TGriffinBgo() = default;

   TGriffinBgo& operator=(const TGriffinBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TGriffinBgo, 1) // GriffinBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
