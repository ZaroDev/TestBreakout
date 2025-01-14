/*
MIT License

Copyright (c) 2023 V�ctor Falc�n Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <Common.h>
#include "MathTypes.h"

namespace Math
{
	
	//! Creates a transform matrix from each component
	//! @param translation Translation component
	//! @param rotation Rotation component 
	//! @param scale Scale component
	//! @return Transformation matrix
	NODISCARD Mat4 createTransform(const V3& translation, const Quaternion& rotation, const V3& scale);

    //! Creates a transform matrix from each component
    //! @param translation Translation component
    //! @param rotation Rotation component
    //! @param scale Scale component
    //! @return Transformation matrix
    NODISCARD Mat4 createSpriteTransform(const V3& translation, const Quaternion& rotation, const V3& scale);

    //! Decomposes a given transform into it's components
    //! @param matrix Matrix to descompose
    //! @param translation Translation component
    //! @param rotation Rotation component
    //! @param scale Scale component
	void decomposeTransform(const Mat4& matrix, V3& translation, Quaternion& rotation, V3& scale);

	NODISCARD float lerp(float a, float b, float f);
}