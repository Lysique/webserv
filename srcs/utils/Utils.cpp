/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 14:29:59 by tamighi           #+#    #+#             */
/*   Updated: 2022/05/10 14:33:25 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

std::string	Utils::trim(std::string& line, const std::string& whitespace)
{
	size_t	line_begin = line.find_first_not_of(whitespace);
	if (line_begin == std::string::npos)
		return ("");
	size_t	line_range = line.find_last_not_of(whitespace) - line_begin + 1;

	return (line.substr(line_begin, line_range));
}

bool	Utils::isValidMethod(const std::string& word)
{
	if (word == "GET" || word == "POST" || word == "HEAD" || word == "PUT"
			|| word == "DELETE" || word == "OPTIONS" || word == "TRACE")
		return true;
	return false;
}
