#include "testing/testing.hpp"

#include "geometry/simplification.hpp"

#include "indexer/scales.hpp"

#include "geometry/distance.hpp"
#include "geometry/point2d.hpp"

#include "base/logging.hpp"
#include "base/macros.hpp"
#include "base/stl_add.hpp"

#include "std/limits.hpp"
#include "std/vector.hpp"

namespace
{

typedef m2::PointD P;
typedef m2::DistanceToLineSquare<m2::PointD> DistanceF;
typedef BackInsertFunctor<vector<m2::PointD> > PointOutput;
typedef void (* SimplifyFn)(m2::PointD const *, m2::PointD const *, double,
                            DistanceF, PointOutput);

struct LargePolylineTestData
{
  static m2::PointD const * m_Data;
  static size_t m_Size;
};

void TestSimplificationSmoke(SimplifyFn simplifyFn)
{
  m2::PointD const points[] = { P(0.0, 1.0), P(2.2, 3.6), P(3.2, 3.6)  };
  double const epsilon = 0.1;
  vector<m2::PointD> result, expectedResult(points, points + 3);
  simplifyFn(points, points + 3, epsilon, DistanceF(), MakeBackInsertFunctor(result));
  TEST_EQUAL(result, expectedResult, (epsilon));
}

void TestSimplificationOfLine(SimplifyFn simplifyFn)
{
  m2::PointD const points[] = { P(0.0, 1.0), P(2.2, 3.6) };
  for (double epsilon = numeric_limits<double>::denorm_min(); epsilon < 1000; epsilon *= 2)
  {
    vector<m2::PointD> result, expectedResult(points, points + 2);
    simplifyFn(points, points + 2, epsilon, DistanceF(), MakeBackInsertFunctor(result));
    TEST_EQUAL(result, expectedResult, (epsilon));
  }
}

void TestSimplificationOfPoly(m2::PointD const * points, size_t count, SimplifyFn simplifyFn)
{
  for (double epsilon = 0.00001; epsilon < 0.11; epsilon *= 10)
  {
    vector<m2::PointD> result;
    simplifyFn(points, points + count, epsilon, DistanceF(), MakeBackInsertFunctor(result));
    // LOG(LINFO, ("eps:", epsilon, "size:", result.size()));

    TEST_GREATER(result.size(), 1, ());
    TEST_EQUAL(result.front(), points[0], (epsilon));
    TEST_EQUAL(result.back(), points[count - 1], (epsilon));
    TEST_LESS(result.size(), count, (epsilon));
  }
}

}

UNIT_TEST(Simplification_TestDataIsCorrect)
{
  TEST_GREATER_OR_EQUAL(LargePolylineTestData::m_Size, 3, ());
  // LOG(LINFO, ("Polyline test size:", LargePolylineTestData::m_Size));
}

UNIT_TEST(Simplification_DP_Smoke)
{
  TestSimplificationSmoke(&SimplifyDP<DistanceF>);
}

UNIT_TEST(Simplification_DP_Line)
{
  TestSimplificationOfLine(&SimplifyDP<DistanceF>);
}

UNIT_TEST(Simplification_DP_Polyline)
{
  TestSimplificationOfPoly(LargePolylineTestData::m_Data, LargePolylineTestData::m_Size,
                           &SimplifyDP<DistanceF>);
}

namespace
{

void SimplifyNearOptimal10(m2::PointD const * f, m2::PointD const * l, double e,
                           DistanceF dist, PointOutput out)
{
  SimplifyNearOptimal(10, f, l, e, dist, out);
}
void SimplifyNearOptimal20(m2::PointD const * f, m2::PointD const * l, double e,
                           DistanceF dist, PointOutput out)
{
  SimplifyNearOptimal(20, f, l, e, dist, out);
}

}

UNIT_TEST(Simplification_Opt_Smoke)
{
  TestSimplificationSmoke(&SimplifyNearOptimal10);
}

UNIT_TEST(Simplification_Opt_Line)
{
  TestSimplificationOfLine(&SimplifyNearOptimal10);
}

UNIT_TEST(Simplification_Opt10_Polyline)
{
  TestSimplificationOfPoly(LargePolylineTestData::m_Data, LargePolylineTestData::m_Size,
                           &SimplifyNearOptimal10);
}

UNIT_TEST(Simplification_Opt20_Polyline)
{
  TestSimplificationOfPoly(LargePolylineTestData::m_Data, LargePolylineTestData::m_Size,
                           &SimplifyNearOptimal20);
}

namespace
{
  void CheckDPStrict(P const * arr, size_t n, double eps, size_t expectedCount)
  {
    vector<P> vec;
    DistanceF dist;
    SimplifyDP(arr, arr + n, eps, dist,
      AccumulateSkipSmallTrg<DistanceF, P>(dist, vec, eps));

    TEST_GREATER(vec.size(), 1, ());
    TEST_EQUAL(arr[0], vec.front(), ());
    TEST_EQUAL(arr[n-1], vec.back(), ());

    if (expectedCount > 0)
    {
      TEST_EQUAL(expectedCount, vec.size(), ());
    }

    for (size_t i = 2; i < vec.size(); ++i)
    {
      DistanceF d;
      d.SetBounds(vec[i-2], vec[i]);
      TEST_GREATER_OR_EQUAL(d(vec[i-1]), eps, ());
    }
  }
}

UNIT_TEST(Simpfication_DP_DegenerateTrg)
{
  P arr1[] = { P(0, 0), P(100, 100), P(100, 500), P(0, 600) };
  CheckDPStrict(arr1, ARRAY_SIZE(arr1), 1.0, 4);

  P arr2[] = {
    P(0, 0), P(100, 100),
    P(100.1, 150), P(100.2, 200), P(100.3, 250), P(100.4, 300), P(100.3, 350), P(100.2, 400),
    P(100.1, 450), P(100, 500), P(0, 600)
  };
  CheckDPStrict(arr2, ARRAY_SIZE(arr2), 1.0, 4);
}

#include "geometry/geometry_tests/large_polygon.hpp"

m2::PointD const * LargePolylineTestData::m_Data = LargePolygon::kLargePolygon;
size_t LargePolylineTestData::m_Size = ARRAY_SIZE(LargePolygon::kLargePolygon);
