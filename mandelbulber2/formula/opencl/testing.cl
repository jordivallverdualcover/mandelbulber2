/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * asurf trial  beta
 * amazing surf from Mandelbulber3D. Formula proposed by Kali, with features added by Darkbeam
 *
 * Note for the origional version apply a c.x c.y SWAP
 *
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "TestingIteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 TestingIteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL colorAdd = 0.0f;
	REAL4 c = aux->const_c;
	REAL4 oldZ = z;
	bool functionEnabledN[5] = {fractal->transformCommon.functionEnabledAx,
		fractal->transformCommon.functionEnabledAyFalse,
		fractal->transformCommon.functionEnabledAzFalse,
		fractal->transformCommon.functionEnabledBxFalse,
		fractal->transformCommon.functionEnabledByFalse};
	int startIterationN[5] = {fractal->transformCommon.startIterationsA,
		fractal->transformCommon.startIterationsB, fractal->transformCommon.startIterationsC,
		fractal->transformCommon.startIterationsD, fractal->transformCommon.startIterationsE};
	int stopIterationN[5] = {fractal->transformCommon.stopIterationsA,
		fractal->transformCommon.stopIterationsB, fractal->transformCommon.stopIterationsC,
		fractal->transformCommon.stopIterationsD, fractal->transformCommon.stopIterationsE};
	enumMulti_orderOfFoldsCl foldN[5] = {fractal->surfFolds.orderOfFolds1,
		fractal->surfFolds.orderOfFolds2, fractal->surfFolds.orderOfFolds3,
		fractal->surfFolds.orderOfFolds4, fractal->surfFolds.orderOfFolds5};

	for (int f = 0; f < 5; f++)
	{
		if (functionEnabledN[f] && aux->i >= startIterationN[f] && aux->i < stopIterationN[f])
		{
			switch (foldN[f])
			{
				case multi_orderOfFoldsCl_type1: // tglad fold
				default:
					z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
								- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
					z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
								- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
					if (z.x != oldZ.x) colorAdd += fractal->mandelbox.color.factor.x;
					if (z.y != oldZ.y) colorAdd += fractal->mandelbox.color.factor.y;
					//					if (z.x != oldZ.x) colorAdd += fractal->foldColor.factor000.x;
					//					if (z.y != oldZ.y) colorAdd += fractal->foldColor.factor000.y;
					break;
				case multi_orderOfFoldsCl_type2: // z = fold - fabs( fabs(z) - fold)
					z.x = fractal->transformCommon.additionConstant111.x
								- fabs(fabs(z.x) - fractal->transformCommon.offset111.x);
					z.y = fractal->transformCommon.additionConstant111.y
								- fabs(fabs(z.y) - fractal->transformCommon.offset111.y);
					if (z.x != oldZ.x) colorAdd += fractal->mandelbox.color.factor.x;
					if (z.y != oldZ.y) colorAdd += fractal->mandelbox.color.factor.y;
					break;
				case multi_orderOfFoldsCl_type3:
					// z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
					z.x = fractal->transformCommon.offset2
								- fabs(fabs(z.x + fractal->transformCommon.offsetA111.x)
											 - fractal->transformCommon.offset2)
								- fractal->transformCommon.offsetA111.x;
					z.y = fractal->transformCommon.offset2
								- fabs(fabs(z.y + fractal->transformCommon.offsetA111.y)
											 - fractal->transformCommon.offset2)
								- fractal->transformCommon.offsetA111.y;
					if (z.x != oldZ.x) colorAdd += fractal->mandelbox.color.factor.x;
					if (z.y != oldZ.y) colorAdd += fractal->mandelbox.color.factor.y;
					//					if (z.x != oldZ.x) colorAdd += fractal->foldColor.factor000.x;
					//					if (z.y != oldZ.y) colorAdd += fractal->foldColor.factor000.y;
					break;
					/*case multi_orderOfFoldsCl_type4:
						// if z > limit) z =  Value -z,   else if z < limit) z = - Value - z,
						if (fabs(z.x) > fractal->transformCommon.additionConstant111.x)
						{
							z.x = mad(sign(z.x), fractal->mandelbox.foldingValue, -z.x);
//							colorAdd += fractal->mandelbox.color.factor.x;
							colorAdd += fractal->foldColor.factor000.x;
						}
						if (fabs(z.y) > fractal->transformCommon.additionConstant111.y)
						{
							z.y = mad(sign(z.y), fractal->mandelbox.foldingValue, -z.y);
//							colorAdd += fractal->mandelbox.color.factor.y;
							colorAdd += fractal->foldColor.factor000.x;
						}
						break;
					case multi_orderOfFoldsCl_type5:
						// z = fold2 - fabs( fabs(z + fold) - fold2) - fabs(fold)
						z.x = fractal->transformCommon.offset2
									- fabs(fabs(z.x + fractal->transformCommon.additionConstant111.x)
												 - fractal->transformCommon.offset2)
									- fractal->transformCommon.additionConstant111.x;
						z.y = fractal->transformCommon.offset2
									- fabs(fabs(z.y + fractal->transformCommon.additionConstant111.y)
												 - fractal->transformCommon.offset2)
									- fractal->transformCommon.additionConstant111.y;
//						if (z.x != oldZ.x) colorAdd += fractal->mandelbox.color.factor.x;
//						if (z.y != oldZ.y) colorAdd += fractal->mandelbox.color.factor.y;
						if (z.x != oldZ.x) colorAdd += fractal->foldColor.factor000.x;
						if (z.y != oldZ.y) colorAdd += fractal->foldColor.factor000.y;
						break;*/
			}
		}
	}

	/*{
		z = fabs(z + fractal->transformCommon.additionConstant000)
				- fabs(z - fractal->transformCommon.additionConstant000) - z;

		if (fractal->transformCommon.functionEnabledFalse
				&& aux->i >= fractal->transformCommon.startIterationsA
				&& aux->i < fractal->transformCommon.stopIterationsA)
		{
			REAL4 limit = fractal->transformCommon.additionConstant000;
			REAL4 length = 2.0f * limit;
			REAL4 tgladS = native_recip(length);
			REAL4 Add;
			if (fabs(z.x) < limit.x) Add.x = z.x * z.x * tgladS.x;
			if (fabs(z.y) < limit.y) Add.y = z.y * z.y * tgladS.y;
			if (fabs(z.z) < limit.z) Add.z = z.z * z.z * tgladS.z;

			if (fabs(z.x) > limit.x && fabs(z.x) < length.x)
				Add.x = (length.x - fabs(z.x)) * (length.x - fabs(z.x)) * tgladS.x;
			if (fabs(z.y) > limit.y && fabs(z.y) < length.y)
				Add.y = (length.y - fabs(z.y)) * (length.y - fabs(z.y)) * tgladS.y;
			if (fabs(z.z) > limit.z && fabs(z.z) < length.z)
				Add.z = (length.z - fabs(z.z)) * (length.z - fabs(z.z)) * tgladS.z;
			Add *= fractal->transformCommon.offset000;
			z.x = (z.x - (sign(z.x) * (Add.x)));
			z.y = (z.y - (sign(z.y) * (Add.y)));
			z.z = (z.z - (sign(z.z) * (Add.z)));
		}
	}*/
	if (fractal->transformCommon.functionEnabledAxFalse)
	{
		z.z = fabs(z.z + fractal->transformCommon.additionConstant111.z)
					- fabs(z.z - fractal->transformCommon.additionConstant111.z) - z.z;
		if (z.z != oldZ.z) colorAdd += fractal->mandelbox.color.factor.z;
		//		if (z.y != oldZ.z) colorAdd += fractal->foldColor.factor000.z;
	}

	// swap
	if (fractal->transformCommon.functionEnabledSwFalse)
	{
		z = (REAL4){z.y, z.x, z.z, z.w};
	}

	z += fractal->transformCommon.additionConstant000;

	// standard functions
	if (fractal->transformCommon.functionEnabledAy)
	{
		REAL r2;
		r2 = dot(z, z);
		if (fractal->transformCommon.functionEnabledFalse)		// force cylinder fold
			r2 -= z.z * z.z * fractal->transformCommon.scaleB1; // fold weight  ;

		// Mandelbox Spherical fold
		if (aux->i >= fractal->transformCommon.startIterationsM
				&& aux->i < fractal->transformCommon.stopIterationsM)
		{

			z += fractal->mandelbox.offset;

			// if (r2 < 1e-21f) r2 = 1e-21f;
			if (r2 < fractal->transformCommon.minR2p25)
			{
				REAL tglad_factor1 =
					native_divide(fractal->transformCommon.maxR2d1, fractal->transformCommon.minR2p25);
				z *= tglad_factor1;
				aux->DE *= tglad_factor1;
				colorAdd += fractal->mandelbox.color.factorSp1;
				//				colorAdd += fractal->foldColor.factorMinR0;
			}
			else if (r2 < fractal->transformCommon.maxR2d1)
			{
				REAL tglad_factor2 = native_divide(fractal->transformCommon.maxR2d1, r2);
				z *= tglad_factor2;
				aux->DE *= tglad_factor2;
				colorAdd += fractal->mandelbox.color.factorSp2;
				//				colorAdd += fractal->foldColor.factorMaxR0;
			}
			z -= fractal->mandelbox.offset;
		}

		if (aux->i >= fractal->transformCommon.startIterationsS
				&& aux->i < fractal->transformCommon.stopIterationsS)
		{ // scale
			z *= fractal->mandelbox.scale;
			aux->DE = mad(aux->DE, fabs(fractal->mandelbox.scale), 1.0f);
		}
	}
	if (fractal->transformCommon.addCpixelEnabledFalse)
	{
		REAL4 tempC = c;
		if (fractal->transformCommon.alternateEnabledFalse) // alternate
		{
			tempC = aux->c;
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZCl_xyz:
				default: tempC = (REAL4){tempC.x, tempC.y, tempC.z, tempC.w}; break;
				case multi_OrderOfXYZCl_xzy: tempC = (REAL4){tempC.x, tempC.z, tempC.y, tempC.w}; break;
				case multi_OrderOfXYZCl_yxz: tempC = (REAL4){tempC.y, tempC.x, tempC.z, tempC.w}; break;
				case multi_OrderOfXYZCl_yzx: tempC = (REAL4){tempC.y, tempC.z, tempC.x, tempC.w}; break;
				case multi_OrderOfXYZCl_zxy: tempC = (REAL4){tempC.z, tempC.x, tempC.y, tempC.w}; break;
				case multi_OrderOfXYZCl_zyx: tempC = (REAL4){tempC.z, tempC.y, tempC.x, tempC.w}; break;
			}
			aux->c = tempC;
		}
		else
		{
			switch (fractal->mandelbulbMulti.orderOfXYZ)
			{
				case multi_OrderOfXYZCl_xyz:
				default: tempC = (REAL4){c.x, c.y, c.z, c.w}; break;
				case multi_OrderOfXYZCl_xzy: tempC = (REAL4){c.x, c.z, c.y, c.w}; break;
				case multi_OrderOfXYZCl_yxz: tempC = (REAL4){c.y, c.x, c.z, c.w}; break;
				case multi_OrderOfXYZCl_yzx: tempC = (REAL4){c.y, c.z, c.x, c.w}; break;
				case multi_OrderOfXYZCl_zxy: tempC = (REAL4){c.z, c.x, c.y, c.w}; break;
				case multi_OrderOfXYZCl_zyx: tempC = (REAL4){c.z, c.y, c.x, c.w}; break;
			}
		}
		z += tempC * fractal->transformCommon.constantMultiplier111;
	}
	if (fractal->mandelbox.mainRotationEnabled && aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
		z = Matrix33MulFloat4(fractal->mandelbox.mainRot, z);

	if (fractal->foldColor.auxColorEnabled)
	{
		aux->color += colorAdd;
	}
	return z;
}