// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>


#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/util/math.hpp>


/*!
\defgroup buffer buffer: calculate buffer of a geometry
\par Source description:
- OGC: Returns a geometric object that represents all Points whose distance
from this geometric object is less than or equal to distance. Calculations are in the spatial reference system of
this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
small error in this distance, but it should be near the resolution of the coordinates used
\see http://en.wikipedia.org/wiki/Buffer_(GIS)
*/
namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

template <typename BoxIn, typename BoxOut, typename T, std::size_t C, std::size_t D, std::size_t N>
struct box_loop
{
    typedef typename coordinate_type<BoxOut>::type coordinate_type;

    static inline void apply(BoxIn const& box_in, T const& distance, BoxOut& box_out)
    {
        coordinate_type d = distance;
        set<C, D>(box_out, get<C, D>(box_in) + d);
        box_loop<BoxIn, BoxOut, T, C, D + 1, N>::apply(box_in, distance, box_out);
    }
};

template <typename BoxIn, typename BoxOut, typename T, std::size_t C, std::size_t N>
struct box_loop<BoxIn, BoxOut, T, C, N, N>
{
    static inline void apply(BoxIn const&, T const&, BoxOut&) {}
};

// Extends a box with the same amount in all directions
template<typename BoxIn, typename BoxOut, typename T>
inline void buffer_box(BoxIn const& box_in, T const& distance, BoxOut& box_out)
{
    assert_dimension_equal<BoxIn, BoxOut>();

    static const std::size_t N = dimension<BoxIn>::value;

    box_loop<BoxIn, BoxOut, T, min_corner, 0, N>::apply(box_in, -distance, box_out);
    box_loop<BoxIn, BoxOut, T, max_corner, 0, N>::apply(box_in, +distance, box_out);
}



}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename TagIn, typename TagOut, typename Input, typename T, typename Output>
struct buffer {};


template <typename BoxIn, typename T, typename BoxOut>
struct buffer<box_tag, box_tag, BoxIn, T, BoxOut>
{
    static inline void apply(BoxIn const& box_in, T const& distance,
                T const& chord_length, BoxIn& box_out)
    {
        detail::buffer::buffer_box(box_in, distance, box_out);
    }
};

// Many things to do. Point is easy, other geometries require self intersections
// For point, note that it should output as a polygon (like the rest). Buffers
// of a set of geometries are often lateron combined using a "dissolve" operation.
// Two points close to each other get a combined kidney shaped buffer then.

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate buffer (= new geometry) around specified distance of geometry
    \ingroup buffer
    \param geometry_in input geometry
    \param distance the distance used in buffer
    \param chord_length length of the chord's in the generated arcs around points or bends
    \param geometry_out buffered geometry
    \note Currently only implemented for box, the trivial case, but still useful
    \par Use case:
        BOX + distance -> BOX: it is allowed that "geometry_out" the same object as "geometry_in"
 */
template <typename Input, typename Output, typename T>
inline void buffer(Input const& geometry_in, Output& geometry_out,
            T const& distance, T const& chord_length = -1)
{
    concept::check<const Input>();
    concept::check<Output>();

    dispatch::buffer
        <
            typename tag<Input>::type,
            typename tag<Output>::type,
            Input,
            T,
            Output
        >::apply(geometry_in, distance, chord_length, geometry_out);
}

/*!
    \brief Calculate and return buffer (= new geometry) around specified distance of geometry
    \ingroup buffer
    \param geometry input geometry
    \param distance the distance used in buffer
    \param chord_length length of the chord's in the generated arcs around points or bends
    \return the buffered geometry
    \note See also: buffer
 */
template <typename Output, typename Input, typename T>
Output make_buffer(Input const& geometry, T const& distance, T const& chord_length = -1)
{
    concept::check<const Input>();
    concept::check<Output>();

    Output geometry_out;

    dispatch::buffer
        <
            typename tag<Input>::type,
            typename tag<Output>::type,
            Input,
            T,
            Output
        >::apply(geometry, distance, chord_length, geometry_out);

    return geometry_out;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP
