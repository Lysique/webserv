/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/05 11:34:22 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 13:32:28 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESHANDLER_HPP
# define RESHANDLER_HPP

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>

#include "../config/ParserConfig.hpp"
#include "../request/ParserRequest.hpp"
#include "../utils/cgi.hpp"
#include "../server/Server.hpp"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <list>

struct RespHeader
{
	std::string	bando;

	std::string	path;

	std::string	protocol;
	std::string	StatusCode;
	std::string	type;
	std::string	content_type;
	int	content_len;
	std::string	time;
	std::string	http;
	std::string	server;
	std::string	host;
	std::string	encoding;
	std::string	filename;
	std::string	cgitype;
	std::string	serv_name;
	std::string	upload;
	std::string	postname[10000];
	std::string	postvalue[10000];
	char*		files;
	int			code;
	std::map<std::string, std::string> mp;
	int			port;
	int			envj;
	int			max_body;
	std::string	content_length;
	std::string	uril_root;
	std::string	index;
};

class ResHandler
{

public:
	ResHandler(std::vector<ServerMembers> s);
	~ResHandler();

	void	manage_request(RequestMembers r);
private:

	//	Private functions
	void	write_response(void);

	//	Private members
	std::vector<ServerMembers>	servers;
	RequestMembers				request;

public:
	void	ParseQueryString_(const std::string &query_string);
	int	POSTMethodes();
	void	reseat(void);
	int		GetMethodes(void);
	void 	DeleteMethodes(void);
	void	DeletePathHandler(void);
	int		filexist(const char *fileName);
	void parse_post(std::string buf);
	void	GetContent_Type(std::string path);
	void	ErrorPage(void);
	bool CheckifPathisFile(const char * path);

	RespHeader MainServer;
	std::string	TheReposn;

private:
	CGI g;
	std::string	EroRep[520];
	std::string Uploadit(std::string sear, std::string buffer);
	std::string FautoIndex(const char *path);
	std::string geit(std::string const &dirEntry, std::string Directory, std::string const &host);
};
std::ostream&	operator<<(std::ostream &ostr, ParserRequest& pr);

#endif
