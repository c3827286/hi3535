/******************************************************************************
  Some simple Hisilicon HI3531 video input functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "global.h"


/******************************************************************************
* function : Set vpss system memory location
******************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_MemConfig()
{
    HI_CHAR * pcMmzName;
    MPP_CHN_S stMppChnVpss;
    HI_S32 s32Ret, i;

    /*vpss group max is 64, not need config vpss chn.*/
    for(i=0;i<64;i++)
    {
        stMppChnVpss.enModId  = HI_ID_VPSS;
        stMppChnVpss.s32DevId = i;
        stMppChnVpss.s32ChnId = 0;

        if(0 == (i%2))
        {
            pcMmzName = NULL;  
        }
        else
        {
            pcMmzName = "ddr1";
        }

        /*vpss*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVpss, pcMmzName);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Vpss HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    VPSS_CHN_MODE_S stVpssChnMode;
    HI_S32 s32Ret;
    HI_S32 i, j;

    /*** Set Vpss Grp Attr ***/
    if(NULL == pstVpssGrpAttr)
    {
        stGrpAttr.u32MaxW = pstSize->u32Width;
        stGrpAttr.u32MaxH = pstSize->u32Height;
        stGrpAttr.bIeEn = HI_FALSE;
        stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.bHistEn = HI_FALSE;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
        stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }
    

    for(i=0; i<s32GrpCnt; i++)
    {
        VpssGrp = i;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
		
        /*** enable vpss chn, with frame ***/
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = j;
            /* Set Vpss Chn attr */
            stChnAttr.bSpEn = HI_FALSE;
            stChnAttr.bBorderEn = HI_TRUE;
            stChnAttr.stBorder.u32Color = 0xffffff;
            stChnAttr.stBorder.u32LeftWidth = 0;
            stChnAttr.stBorder.u32RightWidth = 0;
            stChnAttr.stBorder.u32TopWidth = 0;
            stChnAttr.stBorder.u32BottomWidth = 0;
			if(getActionAlarmStatus(i)==1)
			{
				#ifdef HI3535
				stVpssChnMode.bDouble 	     = HI_FALSE;
				stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
				stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
				stVpssChnMode.u32Width 	     = 352;
				stVpssChnMode.u32Height 	 = 288;
		       	stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
				
				if(j==2 || j==1)
		        {
			        s32Ret = HI_MPI_VPSS_SetChnMode(i, VpssChn, &stVpssChnMode);
			        if (HI_SUCCESS != s32Ret)
			        {
			            printf("set vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
			            return s32Ret;
			        }
		        }
				#endif
				
			}
			
       		int  u32Depth = 6;
	        s32Ret = HI_MPI_VPSS_SetDepth(i, VpssChn, u32Depth);
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
	            return s32Ret;
	        }
            
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
    
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
		
	
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : disable vi dev
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32GrpCnt, HI_S32 s32ChnCnt)
{
    HI_S32 i, j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;

    for(i=0; i<s32GrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }
    
        s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}
HI_S32 SAMPLE_COMM_DisableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_FALSE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}
HI_S32 SAMPLE_COMM_EnableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_TRUE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */