/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:05 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/12 15:09:35 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
 #ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

#include "../config/ParserConfig.hpp"
#include "../request/ParserRequest.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

class ResponseHandler
{

public:
	//	Public functions
	ResponseHandler(std::vector<ServerMembers> s);
	~ResponseHandler();

	std::string	manage_request(RequestMembers r);

private:
	//	Private functions
	std::string	write_response(void);
	std::string	exec_cgi(std::string file_path, std::string exec_path);


	//	Error code management
	int			check_error_code(std::string &path);

	bool		is_method_allowed(void);
	bool		is_method_implemented(void);

	//	Utils
	std::string	find_file_path(std::string path);
	std::string	retrieve_file(std::string path);
	bool		is_file(std::string path);

	//	Headers utils
	std::string	get_date(void);
	std::string	get_content_type(std::string file);

	//	Autoindex
	std::string	dir_to_html(std::string dir_entry, std::string path, std::string host);
	std::string get_autoindex(std::string path);

	//	Private members
	std::vector<ServerMembers>	servers;
	RequestMembers				request;
	LocationMembers				curr_loc;
	std::map<int, std::string>	ErrorResponses;
};

#endif
