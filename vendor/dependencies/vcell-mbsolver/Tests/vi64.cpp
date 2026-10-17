
#pragma warning (disable: 4244 4267)
#include <cmath>
#include <boost/polygon/voronoi.hpp>
#include <boost/polygon/detail/voronoi_ctypes.hpp>

#include <limits>
#include <gtest/gtest.h>

#include <iostream>
namespace vcellVoronoiImpl {
	namespace bp = boost::polygon;
	namespace bpd = boost::polygon::detail;
    typedef long double floatingType;

    struct FloatingConverter {
        template <typename T>
        floatingType operator( )(const T & x)  const {
			return static_cast<floatingType>(x);
        }
		template <int D>
		floatingType operator( )(const bpd::extended_int<D> & x)  const {
			return x.d( );
        }
		floatingType operator( )(const bpd::extended_int<5> & x)  const {
			return x.d( );
        }
		floatingType operator( )(const bpd::extended_int<128> & x)  const {
			return x.d( );
        }
		
    };

    struct compareFloatingType {
        enum Result {
            LESS = -1,
            EQUAL = 0,
            MORE = 1
        };

        Result operator()(floatingType a, floatingType b, unsigned int maxUlps) const {
            if (a > b) {
                return a - b <= maxUlps ? EQUAL : LESS;
            }
            return b - a  <= maxUlps ? EQUAL : MORE;
        }
    };

    struct voronoi_ctype_traits {
        typedef boost::int64_t int_type;
		typedef bpd::extended_int<4> int_x2_type;
        typedef bpd::extended_int<5> uint_x2_type;
		typedef bpd::extended_int<128> big_int_type;
        typedef floatingType fpt_type;
        typedef floatingType efpt_type;
        typedef compareFloatingType ulp_cmp_type;
        typedef FloatingConverter to_fpt_converter_type;
        typedef FloatingConverter to_efpt_converter_type;
    };

    struct vcell_vd_traits{
        typedef voronoi_ctype_traits::fpt_type coordinate_type;
        typedef bp::voronoi_cell<coordinate_type> cell_type;
        typedef bp::voronoi_vertex<coordinate_type> vertex_type;
        typedef bp::voronoi_edge<coordinate_type> edge_type;

        struct vertex_equality_predicate_type {
            enum { ULPS = 128 };

            bool operator()(const vertex_type &v1, const vertex_type &v2) const {
                return (ulp_cmp(v1.x(), v2.x(), ULPS) == compareFloatingType::EQUAL &&
                    ulp_cmp(v1.y(), v2.y(), ULPS) == compareFloatingType::EQUAL);
            }

        private:
            compareFloatingType ulp_cmp;
        };
    };
}

using namespace vcellVoronoiImpl;

TEST(highv,buildi) {
    bp::voronoi_builder<boost::int64_t, vcellVoronoiImpl::voronoi_ctype_traits> vb;
    bp::voronoi_diagram<boost::int64_t,vcell_vd_traits> vd;
   vb.construct(&vd);
}


