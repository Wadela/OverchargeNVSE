#include "NiTypes.h"
#include <cmath>
#include <numbers>

void NiQuaternion::setFromEuler(float eulerX, float eulerY, float eulerZ) {

    float roll = eulerX * std::numbers::pi / 180.0f;
    float pitch = eulerY * std::numbers::pi / 180.0f;
    float yaw = eulerZ * std::numbers::pi / 180.0f;

    float cy = cos(yaw * 0.5f);
    float sy = sin(yaw * 0.5f);
    float cp = cos(pitch * 0.5f);
    float sp = sin(pitch * 0.5f);
    float cr = cos(roll * 0.5f);
    float sr = sin(roll * 0.5f);

    w = cr * cp * cy + sr * sp * sy;
    x = sr * cp * cy - cr * sp * sy;
    y = cr * sp * cy + sr * cp * sy;
    z = cr * cp * sy - sr * sp * cy;
}

NiQuaternion NiQuaternion::createFromEuler(float eulerX, float eulerY, float eulerZ) {
    // Convert degrees to radians
    float roll = eulerX * std::numbers::pi / 180.0;
    float pitch = eulerY * std::numbers::pi / 180.0;
    float yaw = eulerZ * std::numbers::pi / 180.0;

    // Compute half-angles
    float halfRoll = roll / 2.0f;
    float halfPitch = pitch / 2.0f;
    float halfYaw = yaw / 2.0f;

    float sinRoll = std::sin(halfRoll);
    float cosRoll = std::cos(halfRoll);
    float sinPitch = std::sin(halfPitch);
    float cosPitch = std::cos(halfPitch);
    float sinYaw = std::sin(halfYaw);
    float cosYaw = std::cos(halfYaw);

    // Calculate the quaternion components based on the ZYX rotation sequence
    NiQuaternion q;
    q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    q.x = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    q.y = sinYaw * cosPitch * sinRoll + cosYaw * sinPitch * cosRoll;
    q.z = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

    return q;
}

__declspec(naked) float __vectorcall NiMatrix33::ExtractPitch() const
{
    __asm
    {
        movss	xmm0, [ecx + 0x1C]
        xorps	xmm0, PS_FlipSignMask0
        jmp		ASin
    }
}

__declspec(naked) float __vectorcall NiMatrix33::ExtractRoll() const
{
    __asm
    {
        movups	xmm0, [ecx + 0x18]
        pshufd	xmm1, xmm0, 2
        jmp		ATan2
    }
}

__declspec(naked) float __vectorcall NiMatrix33::ExtractYaw() const
{
    __asm
    {
        movups	xmm0, [ecx + 4]
        pshufd	xmm1, xmm0, 3
        jmp		ATan2
    }
}

void NiMatrix33::ExtractAngles(double& rotX, double& rotY, double& rotZ)
{
    rotY = asin(-cr[0][2]);
    if (cos(rotY) != 0)
    {
        rotX = atan2(cr[1][2], cr[2][2]);
        rotZ = atan2(cr[0][1], cr[0][0]);
    }
    else
    {
        rotX = atan2(-cr[2][0], cr[1][1]);
        rotZ = 0;
    }
}

void NiMatrix33::ExtractAnglesDegrees(double& rotX, double& rotY, double& rotZ)
{
    const double radToDeg = 180.0 / std::numbers::pi;

    double gamma = asin(-cr[0][2]);
    if (cos(gamma) != 0)
    {
        double beta = atan2(cr[1][2], cr[2][2]);
        double alpha = atan2(cr[0][1], cr[0][0]);

        rotX = beta * radToDeg;
        rotY = gamma * radToDeg;
        rotZ = alpha * radToDeg;
    }
    else
    {
        rotX = atan2(-cr[2][0], cr[1][1]) * radToDeg;
        rotY = gamma * radToDeg;
        rotZ = 0;
    }
}

// Constructs a rotation matrix from Euler angles in radians
void NiMatrix33::RotationMatrix(float rotX, float rotY, float rotZ)
{
    NiMatrix33 Rx, Ry, Rz;

    // Rotation matrix for rotation about X-axis
    Rx.cr[0][0] = 1;    Rx.cr[0][1] = 0;          Rx.cr[0][2] = 0;
    Rx.cr[1][0] = 0;    Rx.cr[1][1] = cos(rotX);  Rx.cr[1][2] = -sin(rotX);
    Rx.cr[2][0] = 0;    Rx.cr[2][1] = sin(rotX);  Rx.cr[2][2] = cos(rotX);

    // Rotation matrix for rotation about Y-axis
    Ry.cr[0][0] = cos(rotY);  Ry.cr[0][1] = 0;  Ry.cr[0][2] = sin(rotY);
    Ry.cr[1][0] = 0;          Ry.cr[1][1] = 1;  Ry.cr[1][2] = 0;
    Ry.cr[2][0] = -sin(rotY); Ry.cr[2][1] = 0;  Ry.cr[2][2] = cos(rotY);

    // Rotation matrix for rotation about Z-axis
    Rz.cr[0][0] = cos(rotZ);  Rz.cr[0][1] = -sin(rotZ); Rz.cr[0][2] = 0;
    Rz.cr[1][0] = sin(rotZ);  Rz.cr[1][1] = cos(rotZ);  Rz.cr[1][2] = 0;
    Rz.cr[2][0] = 0;          Rz.cr[2][1] = 0;          Rz.cr[2][2] = 1;

    // Combine rotations
    MultiplyMatrices(Ry, Rx);
    MultiplyMatrices(*this, Rz);
}

// Rotates this matrix by Euler angles in radians
void NiMatrix33::Rotate(float rotX, float rotY, float rotZ)
{
    NiMatrix33 rotMat;
    rotMat.RotationMatrix(rotX, rotY, rotZ);
    MultiplyMatrices(*this, rotMat);
}

void NiMatrix33::RotationMatrixDegrees(float rotX, float rotY, float rotZ)
{
    // Convert degrees to radians
    float rotXrad = rotX * std::numbers::pi / 180.0;
    float rotYrad = rotY * std::numbers::pi / 180.0;
    float rotZrad = rotZ * std::numbers::pi / 180.0;

    NiMatrix33 Rx, Ry, Rz;

    // Rotation matrix for rotation about X-axis
    Rx.cr[0][0] = 1;    Rx.cr[0][1] = 0;          Rx.cr[0][2] = 0;
    Rx.cr[1][0] = 0;    Rx.cr[1][1] = cos(rotXrad);  Rx.cr[1][2] = -sin(rotXrad);
    Rx.cr[2][0] = 0;    Rx.cr[2][1] = sin(rotXrad);  Rx.cr[2][2] = cos(rotXrad);

    // Rotation matrix for rotation about Y-axis
    Ry.cr[0][0] = cos(rotYrad);  Ry.cr[0][1] = 0;  Ry.cr[0][2] = sin(rotYrad);
    Ry.cr[1][0] = 0;          Ry.cr[1][1] = 1;  Ry.cr[1][2] = 0;
    Ry.cr[2][0] = -sin(rotYrad); Ry.cr[2][1] = 0;  Ry.cr[2][2] = cos(rotYrad);

    // Rotation matrix for rotation about Z-axis
    Rz.cr[0][0] = cos(rotZrad);  Rz.cr[0][1] = -sin(rotZrad); Rz.cr[0][2] = 0;
    Rz.cr[1][0] = sin(rotZrad);  Rz.cr[1][1] = cos(rotZrad);  Rz.cr[1][2] = 0;
    Rz.cr[2][0] = 0;          Rz.cr[2][1] = 0;          Rz.cr[2][2] = 1;

    // Combine rotations
    MultiplyMatrices(Ry, Rx);
    MultiplyMatrices(*this, Rz);
}

void NiMatrix33::RotateDegrees(float rotX, float rotY, float rotZ)
{
    NiMatrix33 rotMat;
    rotMat.RotationMatrixDegrees(rotX, rotY, rotZ);
    MultiplyMatrices(*this, rotMat);
}

void NiMatrix33::SetRotationFromEulerDegrees(float rotX, float rotY, float rotZ)
{
    // Convert degrees to radians
    float rotXrad = rotX * std::numbers::pi / 180.0;
    float rotYrad = rotY * std::numbers::pi / 180.0;
    float rotZrad = rotZ * std::numbers::pi / 180.0;

    // Reset the current matrix to identity
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            cr[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Rotation matrices for each axis
    NiMatrix33 Rx, Ry, Rz;

    // Rotation matrix for rotation about X-axis
    Rx.cr[0][0] = 1;    Rx.cr[0][1] = 0;           Rx.cr[0][2] = 0;
    Rx.cr[1][0] = 0;    Rx.cr[1][1] = cos(rotXrad);  Rx.cr[1][2] = -sin(rotXrad);
    Rx.cr[2][0] = 0;    Rx.cr[2][1] = sin(rotXrad);  Rx.cr[2][2] = cos(rotXrad);

    // Rotation matrix for rotation about Y-axis
    Ry.cr[0][0] = cos(rotYrad);  Ry.cr[0][1] = 0;  Ry.cr[0][2] = sin(rotYrad);
    Ry.cr[1][0] = 0;             Ry.cr[1][1] = 1;  Ry.cr[1][2] = 0;
    Ry.cr[2][0] = -sin(rotYrad); Ry.cr[2][1] = 0;  Ry.cr[2][2] = cos(rotYrad);

    // Rotation matrix for rotation about Z-axis
    Rz.cr[0][0] = cos(rotZrad);  Rz.cr[0][1] = -sin(rotZrad); Rz.cr[0][2] = 0;
    Rz.cr[1][0] = sin(rotZrad);  Rz.cr[1][1] = cos(rotZrad);  Rz.cr[1][2] = 0;
    Rz.cr[2][0] = 0;             Rz.cr[2][1] = 0;             Rz.cr[2][2] = 1;

    // Combine rotations in order: first X, then Y, finally Z
    MultiplyMatrices(Ry, Rx);
    MultiplyMatrices(*this, Rz);
}

// Multiplies two matrices and stores the result in this matrix
void NiMatrix33::MultiplyMatrices(NiMatrix33& matA, NiMatrix33& matB)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            cr[i][j] = 0;
            for (int k = 0; k < 3; ++k)
            {
                cr[i][j] += matA.cr[i][k] * matB.cr[k][j];
            }
        }
    }
}

void NiMatrix33::Copy(NiMatrix33* toCopy)
{
    this->cr[0][0] = toCopy->cr[0][0];
    this->cr[0][1] = toCopy->cr[0][1];
    this->cr[0][2] = toCopy->cr[0][2];
    this->cr[1][0] = toCopy->cr[1][0];
    this->cr[1][1] = toCopy->cr[1][1];
    this->cr[1][2] = toCopy->cr[1][2];
    this->cr[2][0] = toCopy->cr[2][0];
    this->cr[2][1] = toCopy->cr[2][1];
    this->cr[2][2] = toCopy->cr[2][2];
}

float NiMatrix33::Determinant()
{
    return	this->cr[0][0] * (this->cr[1][1] * this->cr[2][2] - this->cr[1][2] * this->cr[2][1]) -
        this->cr[0][1] * (this->cr[1][0] * this->cr[2][2] - this->cr[1][2] * this->cr[2][0]) +
        this->cr[0][2] * (this->cr[1][0] * this->cr[2][1] - this->cr[1][1] * this->cr[2][0]);
}

void NiMatrix33::Inverse(NiMatrix33& in)
{
    float d = this->Determinant();
    this->cr[0][0] = (in.cr[1][1] * in.cr[2][2] - in.cr[1][2] * in.cr[2][1]) / d;
    this->cr[0][1] = (in.cr[0][2] * in.cr[2][1] - in.cr[0][1] * in.cr[2][2]) / d;
    this->cr[0][2] = (in.cr[0][1] * in.cr[1][2] - in.cr[0][2] * in.cr[1][1]) / d;
    this->cr[1][0] = (in.cr[1][2] * in.cr[2][0] - in.cr[1][0] * in.cr[2][2]) / d;
    this->cr[1][1] = (in.cr[0][0] * in.cr[2][2] - in.cr[0][2] * in.cr[2][0]) / d;
    this->cr[1][2] = (in.cr[0][2] * in.cr[1][0] - in.cr[0][0] * in.cr[1][2]) / d;
    this->cr[2][0] = (in.cr[1][0] * in.cr[2][1] - in.cr[1][1] * in.cr[2][0]) / d;
    this->cr[2][1] = (in.cr[0][1] * in.cr[2][0] - in.cr[0][0] * in.cr[2][1]) / d;
    this->cr[2][2] = (in.cr[0][0] * in.cr[1][1] - in.cr[0][1] * in.cr[1][0]) / d;
}

void NiMatrix33::FromAxisAngle(float theta, float x, float y, float z)
{
    ThisCall(0x4168A0, this, theta, x, y, z);
}

// Dumps the matrix to standard output, optionally prefixed by a title
void NiMatrix33::Dump(const char* title)
{
    if (title)
        printf("%s\n", title);

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            printf("%9.6f ", cr[i][j]);
        }
        printf("\n");
    }
}