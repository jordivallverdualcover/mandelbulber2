/**
 * Mandelbulber v2, a 3D fractal generator       ,=#MKNmMMKmmßMNWy,
 *                                             ,B" ]L,,p%%%,,,§;, "K
 * Copyright (C) 2017 Mandelbulber Team        §R-==%w["'~5]m%=L.=~5N
 *                                        ,=mm=§M ]=4 yJKA"/-Nsaj  "Bw,==,,
 * This file is part of Mandelbulber.    §R.r= jw",M  Km .mM  FW ",§=ß., ,TN
 *                                     ,4R =%["w[N=7]J '"5=],""]]M,w,-; T=]M
 * Mandelbulber is free software:     §R.ß~-Q/M=,=5"v"]=Qf,'§"M= =,M.§ Rz]M"Kw
 * you can redistribute it and/or     §w "xDY.J ' -"m=====WeC=\ ""%""y=%"]"" §
 * modify it under the terms of the    "§M=M =D=4"N #"%==A%p M§ M6  R' #"=~.4M
 * GNU General Public License as        §W =, ][T"]C  §  § '§ e===~ U  !§[Z ]N
 * published by the                    4M",,Jm=,"=e~  §  §  j]]""N  BmM"py=ßM
 * Free Software Foundation,          ]§ T,M=& 'YmMMpM9MMM%=w=,,=MT]M m§;'§,
 * either version 3 of the License,    TWw [.j"5=~N[=§%=%W,T ]R,"=="Y[LFT ]N
 * or (at your option)                   TW=,-#"%=;[  =Q:["V""  ],,M.m == ]N
 * any later version.                      J§"mr"] ,=,," =="""J]= M"M"]==ß"
 *                                          §= "=C=4 §"eM "=B:m|4"]#F,§~
 * Mandelbulber is distributed in            "9w=,,]w em%wJ '"~" ,=,,ß"
 * the hope that it will be useful,                 . "K=  ,=RMMMßM"""
 * but WITHOUT ANY WARRANTY;                            .'''
 * without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * ###########################################################################
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com)
 *
 * c++ - opencl connector for the DOF Phase1 OpenCL renderer
 */

#include "opencl_engine_render_dof_phase2.h"

#include "cimage.hpp"
#include "dof.hpp"
#include "files.h"
#include "fractparams.hpp"
#include "global_data.hpp"
#include "opencl_hardware.h"
#include "progress_text.hpp"

cOpenClEngineRenderDOFPhase2::cOpenClEngineRenderDOFPhase2(cOpenClHardware *_hardware)
		: cOpenClEngine(_hardware)
{
#ifdef USE_OPENCL
	paramsDOF.height = 0;
	paramsDOF.width = 0;
	paramsDOF.height = 0;
	paramsDOF.radius = 0.0;
	paramsDOF.blurOpacity = 0.0;
	numberOfPixels = 0;
	inCLZBufferSorted = nullptr;
	inZBufferSorted = nullptr;
	inCLImageBuffer = nullptr;
	inImageBuffer = nullptr;
	outBuffer = nullptr;
	outCl = nullptr;
	optimalJob.sizeOfPixel = 0; // memory usage doens't depend on job size
	optimalJob.optimalProcessingCycle = 0.5;
#endif
}

cOpenClEngineRenderDOFPhase2::~cOpenClEngineRenderDOFPhase2()
{
#ifdef USE_OPENCL
	if (inCLZBufferSorted) delete inCLZBufferSorted;
	if (inCLImageBuffer) delete inCLImageBuffer;

	if (outCl) delete outCl;

	if (inZBufferSorted) delete[] inZBufferSorted;
	if (inImageBuffer) delete[] inImageBuffer;

	if (outBuffer) delete[] outBuffer;
#endif
}

#ifdef USE_OPENCL

void cOpenClEngineRenderDOFPhase2::ReleaseMemory()
{
	if (inCLZBufferSorted) delete inCLZBufferSorted;
	inCLZBufferSorted = nullptr;

	if (inCLImageBuffer) delete inCLImageBuffer;
	inCLImageBuffer = nullptr;

	if (outCl) delete outCl;
	outCl = nullptr;

	if (inZBufferSorted) delete[] inZBufferSorted;
	inZBufferSorted = nullptr;

	if (inImageBuffer) delete[] inImageBuffer;
	inImageBuffer = nullptr;

	if (outBuffer) delete[] outBuffer;
	outBuffer = nullptr;
}

QString cOpenClEngineRenderDOFPhase2::GetKernelName()
{
	return QString("DOFPhase2");
}

void cOpenClEngineRenderDOFPhase2::SetParameters(const sParamRender *paramRender)
{
	paramsDOF.width = paramRender->imageWidth;
	paramsDOF.height = paramRender->imageHeight;
	paramsDOF.deep =
		paramRender->DOFRadius * (paramRender->imageWidth + paramRender->imageHeight) / 2000.0;
	paramsDOF.neutral = paramRender->DOFFocus;
	paramsDOF.blurOpacity = paramRender->DOFBlurOpacity;
	paramsDOF.maxRadius = paramRender->DOFMaxRadius;

	numberOfPixels = paramsDOF.width * paramsDOF.height;
}

bool cOpenClEngineRenderDOFPhase2::LoadSourcesAndCompile(const cParameterContainer *params)
{
	Q_UNUSED(params);

	programsLoaded = false;
	readyForRendering = false;
	emit updateProgressAndStatus(
		tr("OpenCL DOF - initializing"), tr("Compiling sources for DOF phase 2"), 0.0);

	QString openclPath = systemData.sharedDir + "opencl" + QDir::separator();
	QString openclEnginePath = openclPath + "engines" + QDir::separator();

	QByteArray programEngine;
	// pass through define constants
	programEngine.append("#define USE_OPENCL 1\n");

	QStringList clHeaderFiles;
	clHeaderFiles.append("opencl_typedefs.h"); // definitions of common opencl types
	clHeaderFiles.append("dof_cl.h");					 // main data structures

	for (int i = 0; i < clHeaderFiles.size(); i++)
	{
		programEngine.append("#include \"" + openclPath + clHeaderFiles.at(i) + "\"\n");
	}

	QString engineFileName = "dof_phase2.cl";
	QString engineFullFileName = openclEnginePath + engineFileName;
	programEngine.append(LoadUtf8TextFromFile(engineFullFileName));

	// building OpenCl kernel
	QString errorString;

	QElapsedTimer timer;
	timer.start();
	if (Build(programEngine, &errorString))
	{
		programsLoaded = true;
	}
	else
	{
		programsLoaded = false;
		WriteLog(errorString, 0);
	}
	WriteLogDouble(
		"cOpenClEngineRenderDOFPhase2: Opencl DOF build time [s]", timer.nsecsElapsed() / 1.0e9, 2);

	return programsLoaded;
}

bool cOpenClEngineRenderDOFPhase2::PreAllocateBuffers(const cParameterContainer *params)
{
	Q_UNUSED(params);

	cl_int err;

	if (hardware->ContextCreated())
	{

		// output buffer
		size_t buffSize = numberOfPixels * sizeof(cl_float4);
		if (outBuffer) delete[] outBuffer;
		outBuffer = new cl_float4[numberOfPixels];

		if (outCl) delete outCl;
		outCl = new cl::Buffer(
			*hardware->getContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buffSize, outBuffer, &err);
		if (!checkErr(
					err, "*context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buffSize, outBuffer, &err"))
		{
			emit showErrorMessage(
				QObject::tr("OpenCL %1 cannot be created!").arg(QObject::tr("output buffer")),
				cErrorMessage::errorMessage, nullptr);
			return false;
		}

		// input z-buffer
		if (inZBufferSorted) delete[] inZBufferSorted;
		inZBufferSorted = new sSortedZBufferCl[numberOfPixels];

		if (inCLZBufferSorted) delete inCLZBufferSorted;
		inCLZBufferSorted =
			new cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				numberOfPixels * sizeof(sSortedZBufferCl), inZBufferSorted, &err);
		if (!checkErr(err,
					"Buffer::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "
					"sizeof(sClInBuff), inZBuffer, &err)"))
		{
			emit showErrorMessage(
				QObject::tr("OpenCL %1 cannot be created!").arg(QObject::tr("buffer for zBuffer")),
				cErrorMessage::errorMessage, nullptr);
			return false;
		}

		// input image
		if (inImageBuffer) delete[] inImageBuffer;
		inImageBuffer = new cl_float4[numberOfPixels];

		if (inCLImageBuffer) delete inCLImageBuffer;
		inCLImageBuffer =
			new cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				numberOfPixels * sizeof(cl_float4), inImageBuffer, &err);
		if (!checkErr(err,
					"Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numberOfPixels "
					"* sizeof(cl_float4), inImageBuffer, &err)"))
		{
			emit showErrorMessage(
				QObject::tr("OpenCL %1 cannot be created!").arg(QObject::tr("buffer for inImageBuffer")),
				cErrorMessage::errorMessage, nullptr);
			return false;
		}
	}
	else
	{
		emit showErrorMessage(
			QObject::tr("OpenCL context is not ready"), cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderDOFPhase2::AssignParametersToKernel()
{
	int err = kernel->setArg(0, *inCLZBufferSorted); // input data in global memory
	if (!checkErr(err, "kernel->setArg(0, *inCLZBufferSorted)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot set OpenCL argument for %1").arg(QObject::tr("Z-Buffer data")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(1, *inCLImageBuffer); // input data in global memory
	if (!checkErr(err, "kernel->setArg(0, *inCLImageBuffer)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot set OpenCL argument for %1").arg(QObject::tr("input image data")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(2, *outCl); // output buffer
	if (!checkErr(err, "kernel->setArg(1, *outCl)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot set OpenCL argument for %1").arg(QObject::tr("output data")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(3, paramsDOF); // pixel offset
	if (!checkErr(err, "kernel->setArg(2, pixelIndex)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot set OpenCL argument for %1").arg(QObject::tr("DOF params")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderDOFPhase2::WriteBuffersToQueue()
{
	cl_int err = queue->enqueueWriteBuffer(
		*inCLZBufferSorted, CL_TRUE, 0, numberOfPixels * sizeof(sSortedZBufferCl), inZBufferSorted);

	if (!checkErr(err, "CommandQueue::enqueueWriteBuffer(inZBufferSorted)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot enqueue writing OpenCL %1").arg(QObject::tr("input z buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "CommandQueue::finish() - inZBufferSorted"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot finish writing OpenCL %1").arg(QObject::tr("input z buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->enqueueWriteBuffer(
		*inCLImageBuffer, CL_TRUE, 0, numberOfPixels * sizeof(cl_float4), inImageBuffer);

	if (!checkErr(err, "CommandQueue::enqueueWriteBuffer(inImageBuffer)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot enqueue writing OpenCL %1").arg(QObject::tr("input image buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "CommandQueue::finish() - inImageBuffer"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot finish writing OpenCL %1").arg(QObject::tr("input image buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err =
		queue->enqueueWriteBuffer(*outCl, CL_TRUE, 0, numberOfPixels * sizeof(cl_float4), outBuffer);

	if (!checkErr(err, "CommandQueue::enqueueWriteBuffer(outBuffer)"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot enqueue writing OpenCL %1").arg(QObject::tr("output image buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "CommandQueue::finish() - outBuffer"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot finish writing OpenCL %1").arg(QObject::tr("output image buffers")),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderDOFPhase2::ProcessQueue(qint64 pixelsLeft, qint64 pixelIndex)
{
	qint64 limitedWorkgroupSize = optimalJob.workGroupSize;
	qint64 stepSize = optimalJob.stepSize;

	if (optimalJob.stepSize > pixelsLeft)
	{
		size_t mul = pixelsLeft / optimalJob.workGroupSize;
		if (mul > 0)
		{
			stepSize = mul * optimalJob.workGroupSize;
		}
		else
		{
			// in this case will be limited workGroupSize
			stepSize = pixelsLeft;
			limitedWorkgroupSize = pixelsLeft;
		}
	}
	optimalJob.stepSize = stepSize;

	cl_int err = queue->enqueueNDRangeKernel(
		*kernel, cl::NDRange(pixelIndex), cl::NDRange(stepSize), cl::NDRange(limitedWorkgroupSize));
	if (!checkErr(err, "CommandQueue::enqueueNDRangeKernel()"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot enqueue OpenCL rendering jobs"), cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "CommandQueue::finish() - enqueueNDRangeKernel"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot finish rendering DOF"), cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderDOFPhase2::ReadBuffersFromQueue()
{
	size_t buffSize = numberOfPixels * sizeof(cl_float4);

	cl_int err = queue->enqueueReadBuffer(*outCl, CL_TRUE, 0, buffSize, outBuffer);
	if (!checkErr(err, "CommandQueue::enqueueReadBuffer()"))
	{
		emit showErrorMessage(QObject::tr("Cannot enqueue reading OpenCL output buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "CommandQueue::finish() - ReadBuffer"))
	{
		emit showErrorMessage(QObject::tr("Cannot finish reading OpenCL output buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderDOFPhase2::Render(
	cImage *image, cPostRenderingDOF::sSortZ<float> *sortedZBuffer, bool *stopRequest)
{
	if (programsLoaded)
	{
		// The image resolution determines the total amount of work
		int width = image->GetWidth();
		int height = image->GetHeight();

		cProgressText progressText;
		progressText.ResetTimer();

		emit updateProgressAndStatus(
			tr("OpenCL - rendering DOF - phase 1"), progressText.getText(0.0), 0.0);

		QElapsedTimer timer;
		timer.start();

		int numberOfPixels = width * height;
		QList<QRect> lastRenderedRects;

		// copy zBuffer and image to input and output buffers
		for (int i = 0; i < numberOfPixels; i++)
		{
			inZBufferSorted[i].i = sortedZBuffer[i].i;
			inZBufferSorted[i].z = sortedZBuffer[i].z;
			sRGBFloat imagePixel = image->GetPostImageFloatPtr()[i];
			float alpha = image->GetAlphaBufPtr()[i] / 65535.0;
			inImageBuffer[i] = cl_float4{imagePixel.R, imagePixel.G, imagePixel.B, alpha};
			outBuffer[i] = cl_float4{imagePixel.R, imagePixel.G, imagePixel.B, alpha};
		}

		// writing data to queue
		if (!WriteBuffersToQueue()) return false;

		// TODO:
		// insert device for loop here
		// requires initialization for all opencl devices
		// requires optimalJob for all opencl devices

		for (qint64 pixelIndex = 0; pixelIndex < width * height; pixelIndex += optimalJob.stepSize)
		{
			size_t pixelsLeft = width * height - pixelIndex;

			// assign parameters to kernel
			if (!AssignParametersToKernel()) return false;

			// processing queue
			if (!ProcessQueue(pixelsLeft, pixelIndex)) return false;

			double percentDone = double(pixelIndex) / (width * height);
			emit updateProgressAndStatus(
				tr("OpenCL - rendering DOF - phase 2"), progressText.getText(percentDone), percentDone);
			gApplication->processEvents();

			if (*stopRequest)
			{
				return false;
			}
		}

		if (!*stopRequest)
		{
			if (!ReadBuffersFromQueue()) return false;

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					cl_float4 imagePixelCl = outBuffer[x + y * width];
					sRGBFloat pixel(imagePixelCl.s[0], imagePixelCl.s[1], imagePixelCl.s[2]);
					unsigned short alpha = imagePixelCl.s[3] * 65535.0;
					image->PutPixelPostImage(x, y, pixel);
					image->PutPixelAlpha(x, y, alpha);
				}
			}

			WriteLogDouble(
				"cOpenClEngineRenderDOFPhase2: OpenCL Rendering time [s]", timer.nsecsElapsed() / 1.0e9, 2);

			WriteLog("image->CompileImage()", 2);
			image->CompileImage();

			if (image->IsPreview())
			{
				WriteLog("image->ConvertTo8bit()", 2);
				image->ConvertTo8bit();
				WriteLog("image->UpdatePreview()", 2);
				image->UpdatePreview();
				WriteLog("image->GetImageWidget()->update()", 2);
				image->GetImageWidget()->update();
			}
		}

		emit updateProgressAndStatus(
			tr("OpenCL - rendering DOF phase 2 finished"), progressText.getText(1.0), 1.0);

		return true;
	}
	else
	{
		return false;
	}
}

size_t cOpenClEngineRenderDOFPhase2::CalcNeededMemory()
{
	return numberOfPixels * sizeof(cl_float4);
}

#endif // USE_OPEMCL
