/* 
 * File:   debug_main.h
 * Author: nosak
 *
 * Created on 2023/05/24, 15:54
 */

#ifndef DEBUG_MAIN_H
#define	DEBUG_MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif


#define INPUT_BUF_SIZE 16

typedef enum{
    INPUT_INIT,
    INPUT_SAVING,
    INPUT_DATA_FIX
            
} INPUT_CHAR_STEP;


#ifdef	__cplusplus
}
#endif

#endif	/* DEBUG_MAIN_H */

