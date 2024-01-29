#ifndef TGRIFFINANGLES_H
#define TGRIFFINANGLES_H

/** \addtogroup Analysis
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
/// 
/// \class TGriffinAngles
///
/// This class provides the number of angles, their values, and the index a 
/// specific angle is at.
/// It is meant to be used for the angular correlation analysis.
///
////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <map>
#include <functional>

#include "TObject.h"

class TGriffinAngles : public TObject {
public:
	TGriffinAngles(double distance, bool folding, bool grouping, bool addback);
	~TGriffinAngles() {}

	int Index(double angle);
	int NumberOfAngles() const { return fAngles.size(); }
	double Angle(int index) const { auto it = fAngles.begin(); std::advance(it, index); return *it; }

	std::set<double>::iterator begin() const { return fAngles.begin(); }
	std::set<double>::iterator end() const { return fAngles.end(); }

	void Print() const;

private:
	double fRounding{0.01}; ///< we consider any angles whose difference is less than this to be equal
	std::set<double, std::function<bool(const double&, const double&)>> fAngles; ///< set of unique angles, when grouping is used, the largest angle of the group is used!
	std::map<double, int> fAngleMap; ///< Maps angles to indices. This is fairly straight forward without grouping, but if grouping is used multiple angles can be mapped to the same index.

	/// \cond CLASSIMP
	ClassDef(TGriffinAngles, 1)
	/// \endcond
};
/*! @} */
#endif
