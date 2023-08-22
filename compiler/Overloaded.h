/*
 * Overloaded.h
 *
 *  Created on: Aug 22, 2023
 *      Author: tooma
 */

#ifndef COMPILER_OVERLOADED_H_
#define COMPILER_OVERLOADED_H_

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif /* COMPILER_OVERLOADED_H_ */
