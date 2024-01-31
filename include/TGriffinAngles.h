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

#include "TNamed.h"

class TGriffinAngles : public TNamed {
public:
	TGriffinAngles(double distance = 145., bool folding = false, bool grouping = false, bool addback = true);
	~TGriffinAngles() {}

	double Distance() { return fDistance; }
	double Folding() { return fFolding; }
	double Grouping() { return fGrouping; }
	double Addback() { return fAddback; }

	int Index(double angle);
	int NumberOfAngles() const { return fAngles.size(); }
	double Angle(int index) const { auto it = fAngles.begin(); std::advance(it, index); return *it; }
	double AverageAngle(int index) const;

	std::set<double>::iterator begin() const { return fAngles.begin(); }
	std::set<double>::iterator end() const { return fAngles.end(); }

	void Print(Option_t* = "") const override;

private:
	double fDistance{145.}; ///< distance of detector from center of array in mmm
	bool fFolding{false}; ///< flag indicating whether we fold our distribution around 90 degree 
	bool fGrouping{false}; ///< flag indicating whether we group close angles together
	bool fAddback{true}; ///< flag indicating whether we use addback
	double fRounding{0.01}; ///< we consider any angles whose difference is less than this to be equal
	std::set<double> fAngles; ///< set of unique angles, when grouping is used, the largest angle of the group is used!
	std::map<double, int> fAngleMap; ///< Maps angles to indices. This is fairly straight forward without grouping, but if grouping is used multiple angles can be mapped to the same index.

	/// \cond CLASSIMP
	ClassDefOverride(TGriffinAngles, 2)
	/// \endcond
};
/*! @} */
#endif
