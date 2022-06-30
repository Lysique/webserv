/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 14:30:43 by tamighi           #+#    #+#             */
/*   Updated: 2022/06/27 13:56:26 by fejjed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
class Utils
{
public:
	static std::string	trim(std::string& line, const std::string& whitespace = " \t");
	static bool			isValidMethod(const std::string& word);
	int long  autoindx[65535];
};

#endif
