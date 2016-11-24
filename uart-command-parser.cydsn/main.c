/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include "../src/ucmd.h"
#include "../src/app.h"

int main()
{
    CyGlobalIntEnable;

    ucmd_start(115200);
    app_init();
    
    for(;;)
    {
        ucmd_run();
    }
}

/* [] END OF FILE */
