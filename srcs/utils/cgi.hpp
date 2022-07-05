/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/05 11:37:41 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/05 11:37:47 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

#include "../config/ParserConfig.hpp"
#include "../request/ParserRequest.hpp"
#include "Utils.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

class Server;

class CGI{
    public:
        CGI();
        ~CGI();

        std::string cgiExecute(std::string location, std::string executable ,std::string	postname[], std::string	postvalue[], int j,std::string m_request, int code);
};

#endif
