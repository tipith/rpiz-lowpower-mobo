/*
 * common.h
 *
 * Created: 30/06/2019 21.03.52
 *  Author: pihlstro
 */ 


#ifndef COMMON_H_
#define COMMON_H_

#define foreach(idxpvar, col) typeof(col[0])* idxpvar; for( idxpvar=col ; idxpvar < (col + (arraylen(col))) ; idxpvar++)
#define arraylen( ary ) ( sizeof(ary)/sizeof(ary[0]) )

#endif /* COMMON_H_ */