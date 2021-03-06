/*
 * MapLR2HRDispField.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: vgupta
 */

#include <MapFilterLR2HRDispField.h>
#include <random>
#include <time.h>
unsigned long int MapFilterLR2HRDisp::ComputeMatrixIndex(ImageType::Pointer image, ImageType::IndexType index)
{
	ImageType::SizeType size;
	size = image->GetLargestPossibleRegion().GetSize();
	unsigned long int N = index[2]*size[0]*size[1] + index[1]*size[0] +index[0];
	return N;
}

void MapFilterLR2HRDisp::ReadMovingImage(ImageType::Pointer image)
{
	m_MovingImage = image;
}


void MapFilterLR2HRDisp::ReadFixedImage(ImageType::Pointer image)
{
	m_fixedImage = image;
}

void MapFilterLR2HRDisp::ReadMaskImage(MaskSpatialImageType::Pointer image)
{
	m_MaskImage = image;
}

void MapFilterLR2HRDisp::ReadDeformationField(DisplacementFieldImageType::Pointer image)
{
	m_DispField = image;
}

vnl_sparse_matrix<float> MapFilterLR2HRDisp::GetHR2LRMatrix()
{
	return m_SpVnl_Col_normalized;
}

vnl_sparse_matrix<float> MapFilterLR2HRDisp::GetLR2HRMatrix()
{
	return m_SpVnl_Row_normalized;
}

void MapFilterLR2HRDisp::ComputeMapWithDefField()
{
	std::cout << "start program " << std::endl;

	DisplacementFieldTransformType::Pointer defFieldTransform = DisplacementFieldTransformType::New();
	defFieldTransform->SetDisplacementField(m_DispField);

	ImageType::SizeType sizeFixed, sizeMoving;

	sizeFixed = m_fixedImage->GetLargestPossibleRegion().GetSize();
	sizeMoving = m_MovingImage->GetLargestPossibleRegion().GetSize();

	unsigned long int NumVoxelsMoving = sizeMoving[0]*sizeMoving[1]*sizeMoving[2];
	unsigned long int NumVoxelsFixed = sizeFixed[0]*sizeFixed[1]*sizeFixed[2];

	vnl_sparse_matrix<float> SpVnl_Row(NumVoxelsFixed, NumVoxelsMoving);
	vnl_sparse_matrix<float> SpVnl_Col(NumVoxelsMoving, NumVoxelsFixed);

	m_Origin_FI = m_fixedImage->GetOrigin();
	m_Origin_MI = m_MovingImage->GetOrigin();

	ImageType::IndexType IndexFinal_FixedImage;

	for (int i =0; i < 3; i++)
	{
		IndexFinal_FixedImage[i] = sizeFixed[i]-1;
	}

	m_fixedImage->TransformIndexToPhysicalPoint(IndexFinal_FixedImage, m_Final_FI);
	int num =0;

	MaskSpatialObjectType::Pointer maskSO = MaskSpatialObjectType::New();
	maskSO->SetImage(m_MaskImage);
	maskSO->Update();

	ImageType::IndexType testIndexLR, testIndexHR;
	testIndexLR[0]=20; testIndexLR[1]=34; testIndexLR[2]=2;
	testIndexHR[0]=56; testIndexHR[1]=120; testIndexHR[2]=181;


//	PointType tempPoint;
//	m_fixedImage->TransformIndexToPhysicalPoint(testIndexHR, tempPoint);
	
	ImageType::Pointer tempImage_Fixed = ImageType::New();
	CopyImage cpImage1;
	cpImage1.CopyScalarImage(m_fixedImage, tempImage_Fixed);


	//std::cout << tempPoint << std::endl;
	std::cout << "First Point " << m_Origin_FI << std::endl;
	std::cout << "LastPoint " << m_Final_FI << std::endl;	

	double r[3];
	long long int seed = time(NULL);
	long long int seed_in;
	long long int seed_out;
	
	srand (time(NULL));

	
	ScalarIterator itMask(m_fixedImage, m_fixedImage->GetLargestPossibleRegion());
	ImageType::SpacingType spacing;

	spacing = m_fixedImage->GetSpacing();
	
	for (itMask.GoToBegin(); !itMask.IsAtEnd(); ++itMask)
	{
		if (itMask.Get() != 0)
		{
		 PointType P_mid, P_min, P_max;
		 m_fixedImage->TransformIndexToPhysicalPoint(itMask.GetIndex(), P_mid);
		 for (int i=0; i < 3; i++)
		 {
			P_min[i] = P_mid[i] - spacing[i]/2;
			P_max[i] = P_mid[i] + spacing[i]/2;
		 }	
			
		//	std::cout << "Mask Index " << std::endl;
			for (int i=0; i < 10; i++)
			{
				PointType P_rand;
				for (int j=0; j < 3; j++)
				{
				double r =  ((double) rand() /(RAND_MAX)) ;
				double temp_add = r*(P_max[j] - P_min[j]);
				P_rand[j] = P_min[j] + temp_add;
				}
				
			      ImageType::IndexType Index_Fixed, Index_Moving;
			      m_fixedImage->TransformPhysicalPointToIndex(P_rand, Index_Fixed);
		              PointType P_moving;
			      P_moving = defFieldTransform->TransformPoint(P_rand);
			     bool flag_M;
			    flag_M = m_MovingImage->TransformPhysicalPointToIndex(P_moving, Index_Moving);
			
			unsigned long int rN, cN;
			rN = ComputeMatrixIndex(m_fixedImage, Index_Fixed);
			
			// std::cout << Index_Fixed << std::endl;
			if( flag_M == 1)
			{
			  cN = ComputeMatrixIndex(m_MovingImage, Index_Moving); 
			
			  SpVnl_Row(rN,cN) = SpVnl_Row(rN,cN) +1 ;
			}		
			
			}
		}
	}			

	
	SpVnl_Col = SpVnl_Row.transpose();
	
	m_SpVnl_Row_normalized.set_size(NumVoxelsFixed, NumVoxelsMoving );
	m_SpVnl_Col_normalized.set_size(NumVoxelsMoving, NumVoxelsFixed);

	typedef vnl_sparse_matrix_pair<float> pair_t;

    for (int i=0; i < NumVoxelsFixed; i++)
    {
        vcl_vector<pair_t> rowM=SpVnl_Row.get_row(i);
 	    vcl_vector< int> rowIndices;
 	    vcl_vector<float> rowValues;

 	    float total_row = SpVnl_Row.sum_row(i);
 	    if (total_row == 0)
 	    {
 	    	total_row =1;
 	    }
 	    else
 	    {
 	    	for ( vcl_vector<pair_t>::const_iterator it = rowM.begin() ; it !=   rowM.end() ; ++it )
 		        {
 		            rowIndices.push_back( it->first );
 		            rowValues.push_back( it->second/total_row );
 		        }
 		}

 	    m_SpVnl_Row_normalized.set_row(i, rowIndices, rowValues);
    }

    for (int i=0; i < NumVoxelsMoving; i++)
    {
        vcl_vector<pair_t> colM=SpVnl_Col.get_row(i);

        vcl_vector<int> rowIndices;
        vcl_vector<float> rowValues;

        float total_row = SpVnl_Col.sum_row(i);

        if (total_row == 0)
            total_row=1;
        for (vcl_vector<pair_t>::const_iterator it = colM.begin(); it != colM.end(); ++it)
        {
            rowIndices.push_back( it->first);
            rowValues.push_back( it->second/total_row);
        }

        m_SpVnl_Col_normalized.set_row(i, rowIndices, rowValues);
    }

}

