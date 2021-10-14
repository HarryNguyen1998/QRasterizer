#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Utils/Helper.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function testing
///////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Compare int types", "[Helper::IsEqual]")
{
    STATIC_REQUIRE(Helper::IsEqual(1, 1) == true);  // assert that the integral version is evaluated at compile time.
}

TEST_CASE("Compare \"almost 1.0\" to 1.0", "[Helper::IsEqual]")
{
    // Close to 1.0, but with rounding errors
    double a{0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1};
    REQUIRE(Helper::IsEqual(a, 1.0) == true);
}

TEST_CASE("Compare \"almost 0.0\" to 0.0", "[Helper::IsEqual]")
{
    double a{0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1};
    REQUIRE(Helper::IsEqual(a - 1.0, 0.0) == true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector testing
///////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Initialize a math vector", "[Math::Vector]")
{
    STATIC_REQUIRE(Vec2i{} == Vec2i(0));
    STATIC_REQUIRE(Vec2i(2) == Vec2i{2, 2});

    constexpr Vec3f v = {1.0f, 2.0f, 3.0f};
    STATIC_REQUIRE(v[1] == 2.0f);
    constexpr Vec3f v2{1.0f};
    STATIC_REQUIRE(v2[1] == 0.0f);

}

TEST_CASE("Operations", "[Math::Vector]")
{
    SECTION("Arithmetic operations on 2-component vector")
    {
        constexpr Vec2i v{-6, 4};
        STATIC_REQUIRE(Vec2i{-5, 6} == v + Vec2i{1, 2});
        STATIC_REQUIRE(Vec2i{9, 0} == Vec2i{3, 4} - v);
        STATIC_REQUIRE(Vec2i{-18, 12} == 3 * v);
        STATIC_REQUIRE(Vec2i{6, -4} == -v);
    }

    SECTION("Arithmetic operations on 3-component vector")
    {
        Vec3f v{-6.9f, 3.1f, 42.71828f};
        v += Vec3f{1.0f, 1.0f, 1.0f};
        REQUIRE(Vec3f{-5.9f, 4.1f, 43.71828f} == v);
        v /= 2.0f;
        REQUIRE(Vec3f{-2.95f, 2.05f, 21.85914f} == v);
        v *= 4.0f;
        REQUIRE(Vec3f{-11.8f, 8.2f, 87.43656f} == v);
        v -= Vec3f{1.0f, 8.0f, 7.0f};
        REQUIRE(Vec3f{-12.8f, 0.2f, 80.43656f} == v);
    }

    SECTION("Dot product")
    {
        constexpr Vec2i vi{-6, 4};
        STATIC_REQUIRE(-10 == Math::Dot(vi, Vec2i{3, 2}));

        constexpr Vec3f vf{-6.9f, 3.1f, 42.7128f};
        REQUIRE( Helper::IsEqual(166.3512f, Math::Dot(vf, Vec3f{2.0f, 3.0f, 4.0f})) );
    }

    SECTION("Cross Product for 3D vectors")
    {
        constexpr Vec3f vf{-6.9f, 3.1f, 42.7128f};
        REQUIRE(Vec3f{-115.7384f, 113.0256f, -26.9f} == Math::Cross(vf, Vec3f{2.0f, 3.0f, 4.0f}));
    }

    SECTION("Length")
    {
        constexpr Vec2i vi{-6, 8};
        STATIC_REQUIRE(100 == Math::LengthSqr(vi));
        REQUIRE(10 == Math::Length(vi));

        constexpr Vec3f vf{-6.9000f, 3.1000f, 42.71280f};
        REQUIRE(1881.60315f == Math::LengthSqr(vf));
        REQUIRE(43.37745f == Math::Length(vf));
    }

    SECTION("Normal")
    {
        constexpr Vec3f vf{3.0f, 4.0f, 5.0f};
        REQUIRE(Vec3f{0.42426f, 0.56568f, 0.70710f} == Math::Normal(vf));
    }
    
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix testing
///////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Initialized to Identity matrix", "[Math::Matrix]")
{
    SECTION("Initialize 3x3 matrix")
    {
        constexpr Mat33i m33iIdentity{
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        };
        STATIC_REQUIRE(Mat33i{} == m33iIdentity);

        constexpr Mat33i m33iNotIdentity{
            2, 0, 0,
            0, 1, 0,
            0, 0, 1,
        };
        STATIC_REQUIRE(Mat33i{} != m33iNotIdentity);
    }

    SECTION("Initialize 4x4 matrix")
    {
        constexpr Mat44f m44fidentity{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        };
        REQUIRE(Mat44f{} == m44fidentity);
    }
}

TEST_CASE("Transpose", "[Math::Matrix]")
{
    constexpr Mat33i mi = {
        5, 6, 7,
        8, 9, 10,
        11, 12, 13};
    constexpr Mat33i miTransposed{
        5, 8, 11,
        6, 9, 12,
        7, 10, 13};

    SECTION("Transpose 3x3 matrix")
    {
        STATIC_REQUIRE(miTransposed == Transpose(mi));
    }
    SECTION("3x3 matrix-matrix multiplication")
    {
        constexpr Mat33i expected{
            110, 164, 218,
            164, 245, 326,
            218, 326, 434};
        STATIC_REQUIRE(expected == (mi * miTransposed));
    }

    constexpr Mat44f mf = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f};
    constexpr Mat44f mfTransposed{
        1.0f, 5.0f, 9.0f, 13.0f,
        2.0f, 6.0f, 10.0f, 14.0f,
        3.0f, 7.0f, 11.0f, 15.0f,
        4.0f, 8.0f, 12.0f, 16.0f};
    SECTION("Tranpose 4x4 matrix")
    {
        REQUIRE(mfTransposed == Transpose(mf));
    }
    SECTION("4x4 matrix-matrix multiplication")
    {
        Mat44f expected{ 30.0f, 70.0f, 110.0f, 150.0f,
            70.0f, 174.0f, 278.0f, 382.0f,
            110.0f, 278.0f, 446.0f, 614.0f,
            150.0f, 382.0f, 614.0f, 846.0f};
        REQUIRE(expected == (mf * mfTransposed));
    }
}

TEST_CASE("Inverse", "[Math::Matrix]")
{
    Mat33f m = {
        30.0f, 0.0f, 20.0f,
        20.0f, 0.0f, -20.0f,
        0.0f, 10.0f, 10.0f };
    SECTION("Inverse of 3x3 matrix")
    {
        Mat33f expected = {
            0.02000f, 0.02000f, 0.00000f,
            -0.02000f, 0.03000f, 0.10000f,
            0.02000f, -0.03000f, 0.00000f
        };
        REQUIRE(expected == Inverse(m));
    }

    Mat44f m2{
        5.0f, 3.0f, 1.0f, 0.0f,
		1.0f, 0.0f, -2.0f, 0.0f,
		1.0f, 2.0f, 5.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, };
    SECTION("Invese of 4x4 matrix")
    {
        Mat44f expected{
            4.00001f, -13.00003f, -6.00001f, 0.00000f,
            -7.00002f, 24.00005f, 11.00002f, 0.00000f,
            2.00000f, -7.00002f, -3.00001f, -0.00000f,
            0.00000f, 0.00000f, 0.00000f, 1.00000f,};
        REQUIRE(expected == Inverse(m2));
    }
}

TEST_CASE("Vector-matrix multiplication", "[Math::Matrix]")
{
    SECTION("2D")
    {
        constexpr Vec3i vi{3, 4, 5};
        constexpr Mat33i m{
            1, 2, 2,
            2, 1, 2,
            1, 2, 3 };
        constexpr Vec3i expected = { 16, 20, 29 };
        STATIC_REQUIRE(expected == MultiplyVecMat(vi, m));
    }

    SECTION("3D")
    {
        Vec3f v{2.0f, 4.0f, 6.0f};
        Mat44f m{};
        m(2, 2) = -1.0f;
        m(2, 3) = -1.0f;
        m(3, 3) = 0.0f;
        Vec3f vResult{2.0f, 4.0f, -6.0f};
        Vec3f pProjected{-0.33333f, -0.66666f, 1.0f};
        REQUIRE(vResult == MultiplyVecMat(v, m));

    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// matrices initialization testing
///////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Useful matrices", "[Math::Matrix]")
{
    constexpr float M_PI = 3.14159265358979f;
    SECTION("Perspective projection matrix")
    {
        Mat44f m{Math::InitPersp(M_PI / 4.0f, 1.0f, 0.1f, 100.0f)};
        Mat44f expected = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0010f, -1.0f,
            0.0f, 0.0f, -0.1001f, 0.0f};
        REQUIRE(expected != m);
    }

    SECTION("Rotation matrix roll")
    {
        Vec3f x = {1.0f, 0.0f, 0.0f};
        Vec3f y = {0.0f, 1.0f, 0.0f};
        Mat44f m{Math::InitRotation(M_PI / 2.0f, 0.0f, 0.0f)};
        float a = std::cos(M_PI / 2.0f);
        REQUIRE(Vec3f{0.0f, 1.0f, 0.0f} == Math::MultiplyVecMat(x, m));
        REQUIRE(Vec3f{-1.0f, 0.0f, 0.0f} == Math::MultiplyVecMat(y, m));
    }
    SECTION("Rotation matrix pitch")
    {
        Vec3f y = {0.0f, 1.0f, 0.0f};
        Vec3f z = {0.0f, 0.0f, 1.0f};
        Mat44f m{Math::InitRotation(0.0f, M_PI / 2.0f, 0.0f)};
        REQUIRE(Vec3f{0.0f, 0.0f, 1.0f} == Math::MultiplyVecMat(y, m));
        REQUIRE(Vec3f{0.0f, -1.0f, 0.0f} == Math::MultiplyVecMat(z, m));
    }
    SECTION("Rotation matrix yaw")
    {
        Vec3f z = {0.0f, 0.0f, 1.0f};
        Vec3f x = {1.0f, 0.0f, 0.0f};
        Mat44f m{Math::InitRotation(0.0f, 0.0f, M_PI / 2.0f)};
        REQUIRE(Vec3f{1.0f, 0.0f, 0.0f} == Math::MultiplyVecMat(z, m));
        REQUIRE(Vec3f{0.0f, 0.0f, -1.0f} == Math::MultiplyVecMat(x, m));
    }

    SECTION("Affine rotation")
    {
        // Up vec is centered around [1 1 1]
        Vec3f y = {1.0f, 2.0f, 1.0f};
        Vec3f z = {1.0f, 1.0f, 2.0f};
        Mat44f m{Math::InitRotation(0.0f, M_PI / 2.0f, 0.0f)};
        REQUIRE(Vec3f{1.0f, 1.0f, 2.0f} == Math::AffineRotation(y, Vec3f{1.0f, 1.0f, 1.0f}, m));
        REQUIRE(Vec3f{1.0f, 0.0f, 1.0f} == Math::AffineRotation(z, Vec3f{1.0f, 1.0f, 1.0f}, m));
    }

}

