
  @verbatim
  ******************************************************************************
  *  
  *           Portions COPYRIGHT 2016 STMicroelectronics                       
  *           Portions Copyright (C) 1994-2011, Thomas G. Lane, Guido Vollbeding          
  *  
  * @file    st_readme.txt 
  * @author  MCD Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          LibJPEG for integration with STM32Cube solution.  
  ******************************************************************************
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  @endverbatim
### 18-November-2016 ###
========================
   + Remove from LibJpeg all the links with FatFs
   + rename template files for read/write operations from 'jdatasrc_conf_template.c/.h'
   to 'jdata_conf_template.c/.h' .

### 23-September-2016 ###
========================
   + Remove from LibJpeg all the links with FatFs
   + Add template files for read/write operations 'jdatasrc_conf_template.c/.h', these
     files have to be copied to the application folder and modified as follows:
            - Rename them to 'jdatasrc_conf.c/.h'
            - Implement read/write functions (example of implementation is provided based on FatFs)

### 23-December-2014 ###
========================
   + jinclude.h: add newline at end of file to fix compilation warning.


### 19-June-2014 ###
====================
   + First customized version of LibJPEG V8d for STM32Cube solution.
   + LibJPEG is preconfigured to support by default the FatFs file system when
     media are processed from a FAT format media
   + The original “jmorecfg.h” and “jconfig.h” files was renamed into “jmorecfg_template.h”
     and “jconfig_template.h”, respectively. Two macros has been added to specify the memory
     allocation/Freeing methods.
     These two last files need to be copied to the application folder and customized
     depending on the application requirements.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
