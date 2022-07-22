/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:05 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/22 17:18:36 by tamighi          ###   ########.fr       */
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

	void		manage_request(int socket, RequestMembers r);

private:

	//	Private functions
	std::string	manage_response(void);
	std::string	make_response(std::string file, int error_code, std::string path);

	void		get_current_loc(void);
	void		write_response(void);
	std::string	exec_cgi(std::string file_path, std::string exec_path);

	//	Error code management
	int			check_method(void);
	bool		is_method_allowed(void);
	bool		is_method_implemented(void);

	int			check_path_access(std::string path);

	//	Utils
	std::string	get_path(std::string path);
	std::string	retrieve_file(std::string path);
	bool		is_file(std::string path);

	std::string	get_date(void);
	std::string	get_content_type(std::string file);

	//	Autoindex
	std::string	dir_to_html(std::string dir_entry, std::string path);
	std::string get_autoindex(std::string fullpath, std::string path);

	//	Private members

	std::vector<ServerMembers>	servers;
	std::map<int, std::string>	http_responses;
	std::map<int, std::string>	ErrorResponses;

	RequestMembers				request;
	LocationMembers				curr_loc;
	int							curr_sock;
};

#endif
