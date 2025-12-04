#ifndef TTIGRESSBGO_H
#define TTIGRESSBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include "Globals.h"
#include "TBgo.h"

class TTigressBgo : public TBgo {
public:
   TTigressBgo();
   TTigressBgo(const TTigressBgo&);
   TTigressBgo(TTigressBgo&&) noexcept = default;
   TTigressBgo& operator=(const TTigressBgo&);   //!<!
   TTigressBgo& operator=(TTigressBgo&&) noexcept = default;
   virtual ~TTigressBgo()                         = default;

   /// \cond CLASSIMP
   ClassDef(TTigressBgo, 1)   // TigressBgo Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
