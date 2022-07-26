/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:05 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/26 09:48:08 by tamighi          ###   ########.fr       */
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
	
	ResponseHandler(ServerMembers s);
	~ResponseHandler();

	void		manage_response(int socket, RequestMembers r);
	bool		is_sent(void);

private:

			//	Private functions

	std::string	get_response(void);
	std::string	make_response(std::string file, int error_code, std::string path);
	void		write_response(void);

	void		get_current_loc(void);
	std::string	manage_post_request(std::string &path);
	void		upload_file(std::string filename, std::string data);

	std::string	exec_cgi(std::string file_path, std::string exec_path);

	//	Error code management
	int			check_method(void);
	bool		is_method_allowed(void);
	bool		is_method_implemented(void);
	int			check_path_access(std::string path);

	std::string	http_error(int error_code);

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

	ServerMembers				server;
	std::string					http_response;
	std::string					cookie;
	std::map<int, std::string>	error_responses;

	RequestMembers				request;
	LocationMembers				curr_loc;
	int							curr_sock;
};

#endif
