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
	TGriffinBgo(TGriffinBgo&&) noexcept = default;
   TGriffinBgo& operator=(const TGriffinBgo&); //!<!
	TGriffinBgo& operator=(TGriffinBgo&&) noexcept = default;
   virtual ~TGriffinBgo() = default;

   /// \cond CLASSIMP
   ClassDef(TGriffinBgo, 1) // GriffinBgo Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
