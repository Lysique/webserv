/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/05 11:35:33 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 15:03:21 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResHandler.hpp"

ResHandler::ResHandler(std::vector<ServerMembers> s)
	: servers(s)
{
	EroRep[100] = "Continue";
	EroRep[200] = "OK";
	EroRep[201] = "Created";
	EroRep[204] = "No Content";
	EroRep[400] = "Bad Request";
	EroRep[403] = "Forbidden";
	EroRep[404] = "Not Found";
	EroRep[405] = "Method Not Allowed";
	EroRep[413] = "Payload Too Large";
	EroRep[500] = "Internal Server Error";
	EroRep[501] = "Not Implemented";
}

ResHandler::~ResHandler()
{
}

void ResHandler::manage_request(RequestMembers r)
{
	request = r;
	MainServer.host = r.host;
	MainServer.protocol = r.protocol;
	MainServer.type = r.method;
	MainServer.path = r.location;
	MainServer.content_len = r.content_length;

	parse_post(MainServer.bando);
	write_response();
}

void	ResHandler::write_response(void)
{
	ServerMembers	current_server;
	LocationMembers	current_location;

	//	Find correct server
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (request.port == servers[i].port)
			current_server = servers[i];
	}

	//	Find correct location
	for (size_t i = 0; i < current_server.locations.size(); ++i)
	{
		if (request.location.find(current_server.locations[i].uri) != std::string::npos)
		{
			current_location = current_server.locations[i];
			break ;
		}
	}
	
	//	ASSIGN BEGIN

	//	Assign location variables to MainServer
	MainServer.uril_root = current_location.root;
	MainServer.max_body = current_location.max_body_size;
	MainServer.upload = current_server.upload;

	//	Check for serv names
	MainServer.serv_name = "Hello";
	//	Cgi paths to mp
	MainServer.mp = current_server.cgi_paths;
	//	Error to mp
	for (std::map<int, std::string>::iterator it = current_location.error_pages.begin(); it != current_location.error_pages.end(); ++it)
		MainServer.mp[std::to_string(it->first)] = it->second;
	//	Check for location / root / index
	
	//	If root == /path/ and file = /file -> To check
	if (request.location.front() == '/')
		request.location.erase(0, 1);

	//	Add index if needed
	MainServer.path = current_location.root + request.location;
	if (MainServer.path == current_location.root)
		MainServer.path += current_location.index.front();

	//	ASSIGN END

	//	!!Allowed methods!
	if (MainServer.type == "GET")
		GetMethodes();
	else if (MainServer.type == "DELETE")
		DeleteMethodes();
	else if (MainServer.type == "POST")
		POSTMethodes();
	//	Unallowed methods
	else
	{
		reseat();
		if (MainServer.type == "POST" || MainServer.type == "DELETE" || MainServer.type == "GET")
			MainServer.code = 405;
		else
			MainServer.code = 501;


		ErrorPage();
		TheReposn += MainServer.protocol + " " + std::to_string(MainServer.code) + " " + EroRep[MainServer.code];
		TheReposn += "\nServer: Webserv /1.0.0";
		TheReposn += "\n\n";
		TheReposn += MainServer.http;
	}
}

template <typename T>
std::string ToString(T numb)
{
	std::stringstream stream;
	stream << numb;
	return stream.str();
}

void ResHandler::parse_post(std::string buf)
{
	std::stringstream ss(buf);
	std::string word;

	while (ss >> word)
	{
		if (word.size() > 9 && word.substr(0, 9) == "filename=")
			MainServer.filename = word.substr(9);
	}
}

std::string removeAll(std::string str, const std::string &from)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from)) != std::string::npos)
	{
		str.erase(start_pos, from.length());
	}
	return str;
}

bool ResHandler::CheckifPathisFile(const char *path)
{
	struct stat	s;

	if (stat(path, &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return false; // it's a director
		else if (s.st_mode & S_IFREG)
			return true; // it's a file
		else
			return true;
	}
	return true;
}

void ResHandler::GetContent_Type(std::string path)
{
	MainServer.content_type = path.substr(path.rfind(".") + 1, path.size() - path.rfind("."));

	MainServer.cgitype = MainServer.content_type;

	if (MainServer.content_type == "html")
		MainServer.content_type = "text/html";
	else if (MainServer.content_type == "xml")
		MainServer.content_type = "text/xml";
	else if (MainServer.content_type == "js")
		MainServer.content_type = "text/javascript";
	else if (MainServer.content_type == "css")
		MainServer.content_type = "text/css";
	else if (MainServer.content_type == ".cpp")
		MainServer.content_type = "text/plain";

	else if (MainServer.content_type == "pdf")
		MainServer.content_type = "application/pdf";
	else if (MainServer.content_type == "doc")
		MainServer.content_type = "application/msword";

	else if (MainServer.content_type == "jpeg" || MainServer.content_type == "jpg" || MainServer.content_type == "pjp" || MainServer.content_type == "jfif" || MainServer.content_type == "pjpeg")
		MainServer.content_type = "image/jpeg";
	else if (MainServer.content_type == "png")
		MainServer.content_type = "image/png";
	else if (MainServer.content_type == "gif")
		MainServer.content_type = "image/gif";
	else if (MainServer.content_type == "ico")
		MainServer.content_type = "image/x-icon";

	else if (MainServer.content_type == "mp4")
		MainServer.content_type = "video/mp4";
	else if (MainServer.content_type == "webm")
		MainServer.content_type = "video/webm";
	else if (MainServer.content_type == "mpeg")
		MainServer.content_type = "video/mpeg";

	else if (MainServer.content_type == "mp3")
		MainServer.content_type = "audio/mpeg";

	else
		MainServer.content_type = "text/html";
}



int ResHandler::filexist(const char *fileName)
{
	std::string line;
	MainServer.code = 200;
	int autoindx[65535];
	std::vector<ServerMembers>::iterator ite = servers.end();
	std::vector<ServerMembers>::iterator it = servers.begin();

	for (int i = 1; it != ite; ++it, ++i)
	{
		autoindx[it->port] = it->autoindex;
	}

	std::ifstream document;

	//	Check if there is a file
	if (CheckifPathisFile(fileName) == true)
	{
		if ((access(fileName, F_OK) == 0))
		{
			document.open(fileName, std::ifstream::in);
			if (document.is_open() == false)
			{
				MainServer.code = 403;
				return MainServer.code;
			}
			std::stringstream test;
			test << document.rdbuf();
			MainServer.http = test.str();
			if (MainServer.cgitype == "py" && MainServer.mp[".py"].find("/usr/bin/python") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/python", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			else if (MainServer.cgitype == "pl" && MainServer.mp[".pl"].find("/usr/bin/perl") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/perl", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			else if (MainServer.cgitype == "php" && MainServer.mp[".php"].find("/usr/bin/php") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/php", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			return MainServer.code;
		}
		else
			MainServer.code = 404;
		return MainServer.code;
	}
	else if (CheckifPathisFile(fileName) == false && MainServer.type == "GET" && autoindx[atoi(MainServer.host.substr(MainServer.host.find(":") + 1).c_str())] == 1)
	{
		MainServer.http += FautoIndex(fileName);
		MainServer.code = 200;
		return MainServer.code;
	}
	else
		MainServer.code = 404;
	return MainServer.code;
}
void ResHandler::ErrorPage(void)
{
	std::string line;
	std::ifstream fahd;
	std::string path;
	if (MainServer.mp[ToString(MainServer.code)] == "")
	{
		path = ToString(realpath(".", NULL)) + "/srcs/Http(Errors)/" + ToString(MainServer.code) + ".html";
	}
	else
		path = ToString(realpath(".", NULL)) + MainServer.mp[ToString(MainServer.code)];
	std::ifstream document;
	if ((access(path.c_str(), F_OK) == 0))
	{
		document.open(path, std::ios::in);
		while (getline(document, line))
		{
			MainServer.http += line + " ";
		}
	}
}

void	check_path(std::string path)
{
	struct stat	s;


	if (stat(path, &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			std::cout << "FILE\n";
	}

	if (access(path.c_str(), F_OK) == 0)
		std::cout << "xx\n";
}

int ResHandler::GetMethodes(void)
{
	//	Need to check path
	
	//	Weird realpath stuff here
	std::string raw_path = realpath(".", NULL);

	std::string path = realpath(".", NULL) + removeAll(MainServer.path, realpath(".", NULL));

	std::cout << MainServer.path << std::endl;
	MainServer.path = path + "/";

	//	Content type
	GetContent_Type(path);

	//	Check if file exist
	filexist(path.c_str());

	check_path(path);
	//	Error page
	ErrorPage();

	ParseQueryString_(MainServer.bando.substr(MainServer.bando.find("\r\n\r\n") + strlen("\r\n\r\n")));

	//	If content length too big
	if (MainServer.content_len >= MainServer.max_body)
	{
		MainServer.code = 413;
		MainServer.http = "";
		ErrorPage();
	}
	else
		MainServer.code = 200;

	if (MainServer.http.find("Status: 500") != std::string::npos)
	{
		MainServer.code = 500;
		MainServer.http = "";
		ErrorPage();
	}

	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\nServer: " + MainServer.serv_name;
	TheReposn += "\nContent-Type: " + MainServer.content_type;
	TheReposn += "\nContent-Length : " + std::to_string(MainServer.http.size() - 1);
	TheReposn += "\nContent-Location: " + MainServer.path;
	TheReposn += "\nTransfer-Encoding: identity" + MainServer.encoding;
	TheReposn += "\n\n";
	TheReposn += MainServer.http;
	return 0;
}

void ResHandler::DeletePathHandler(void)
{
	std::string path = realpath(".", NULL) + MainServer.path;
	if (CheckifPathisFile(path.c_str()) == true)
	{
		if ((access(path.c_str(), F_OK) == 0))
		{
			if (remove(path.c_str()) == 0)
				MainServer.code = 204;
			else
				MainServer.code = 403;
		}
		else
			MainServer.code = 404;
	}
	else
		MainServer.code = 404;
}

void ResHandler::DeleteMethodes(void)
{
	DeletePathHandler();
	if (MainServer.content_len >= MainServer.max_body)
		MainServer.code = 413;
	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\n\n";
	reseat();
	ErrorPage();
	TheReposn += MainServer.http;
}

std::string ResHandler::Uploadit(std::string sear, std::string buffer)
{
	int check_prob = 0;
	std::string mainbuffer(buffer);
	size_t i = mainbuffer.rfind("filename=\"");
	if (i != std::string::npos)
	{
		i += 10;
		size_t j = mainbuffer.find("\"", i);
		if (j != std::string::npos)
		{
			mainbuffer = std::string((mainbuffer.begin() + i), mainbuffer.begin() + j);
		}
	}
	std::string hahiyaupload = "." + MainServer.upload;
	std::string root = "";
	int fd;
	if ("" == hahiyaupload)
		fd = open((hahiyaupload + "/" + mainbuffer).c_str(), O_RDWR | O_CREAT | O_TRUNC, 00777);
	else
	{
		mkdir((hahiyaupload + "/" + "").c_str(), 0755);
		fd = open((hahiyaupload + "/" + "" + "" + mainbuffer).c_str(), O_RDWR | O_CREAT | O_TRUNC, 00777);
	}
	if (fd == -1)
		return (std::string());
	int checkWrite = write(fd, sear.c_str(), sear.size());
	if (checkWrite == 0)
		check_prob = 0;
	else if (checkWrite == -1)
		check_prob = -1;
	close(fd);
	return (mainbuffer);
}
std::string get_right_of_delim(std::string const &str, std::string const &delim)
{
	return str.substr(str.find(delim) + delim.size());
}

void ResHandler::ParseQueryString_(const std::string &query_string)
{
	std::size_t position = 0;
	std::size_t i = 1;
	while (position < query_string.size())
	{
		const std::size_t next_delimiter = query_string.find('&', position);
		std::string query;
		if (next_delimiter == std::string::npos)
		{
			query = query_string.substr(position);
		}
		else
		{
			query = query_string.substr(position, next_delimiter - position);
		}
		const std::size_t pair_delimiter = query.find('=');
		const std::string name = query.substr(0, pair_delimiter);
		if (name.empty())
		{
			return;
		}
		std::string value;
		if (pair_delimiter != std::string::npos)
		{
			value = query.substr(pair_delimiter + 1);
		}
		MainServer.postname[i] = name;
		MainServer.postvalue[i] = value;
		if (next_delimiter == std::string::npos)
		{
			break;
		}
		position = next_delimiter + 1;
		i++;
	}
	MainServer.envj = i;
}

int ResHandler::POSTMethodes()
{
	std::string path = realpath(".", NULL) + removeAll(MainServer.path, realpath(".", NULL));
	MainServer.code = 413;
	ParseQueryString_(MainServer.bando.substr(MainServer.bando.find("\r\n\r\n") + strlen("\r\n\r\n")));
	GetContent_Type(path);
	if (MainServer.path.substr(MainServer.path.find_last_of(".") + 1) == "php")
		MainServer.http = g.cgiExecute("/usr/bin/php", path.c_str(), MainServer.postname, MainServer.postvalue, MainServer.envj, MainServer.bando);
	if (MainServer.path.substr(MainServer.path.find_last_of(".") + 1) == "py")
		MainServer.http = g.cgiExecute("/usr/bin/python", path.c_str(), MainServer.postname, MainServer.postvalue, MainServer.envj, MainServer.bando);
	bool upload = false;
	if (MainServer.content_len <= MainServer.max_body)
	{
		MainServer.code = 200;
		std::string sear(MainServer.bando);
		size_t i = sear.rfind("Content-Type:");
		if (i != std::string::npos)
			i = sear.find("\n", i);
		if (i != std::string::npos)
		{
			size_t j = sear.find("------WebKitFormBoundary", i);
			if (j != std::string::npos)
			{
				upload = true;
				MainServer.http = "<!DOCTYPE htm1l>\n<html>\n<h1>File " + MainServer.filename + "Hs Been Uploaded</h1>";
				MainServer.http = "<a href=" + MainServer.upload + removeAll(MainServer.filename, "\"") + " download>Download File</a>";
				Uploadit(std::string((sear.begin() + i + 3), sear.begin() + j - 2), MainServer.bando);
			}
		}
	}
	if (MainServer.content_len >= MainServer.max_body)
		MainServer.code = 413;
	if (MainServer.http.find("Status: 500") != std::string::npos)
	{
		MainServer.code = 500;
		MainServer.http = "";
		ErrorPage();
	}
	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\nServer: " + MainServer.serv_name;
	TheReposn += "\nContent-Type: " + MainServer.content_type;
	if (upload != true)
		TheReposn += "\nContent-Length : " + std::to_string(MainServer.http.size() - 1); // make a update for upload
	else
		TheReposn += "\nContent-Length : " + ToString(MainServer.content_len);
	TheReposn += "\nContent-Location: " + MainServer.path;
	TheReposn += "\n\n";
	ErrorPage();
	TheReposn += MainServer.http;
	return 0;
}

void ResHandler::reseat(void)
{
	MainServer.http = "";
}

std::string ResHandler::geit(std::string const &dirEntry, std::string Directory, std::string const &host)
{
	std::stringstream ss;
	if (dirEntry != ".." && dirEntry != ".")
		ss << "\t\t<p><a href=\"http://" + host << Directory + "/" + dirEntry + "\">" + dirEntry + "/" + "</a></p>\n";
	return ss.str();
}

std::string ResHandler::FautoIndex(const char *path)
{
	std::string Directory(MainServer.path);
	DIR *dir = opendir(path);
	std::string Autoindex_Page =
		"<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" +
		Directory + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>INDEX</h1>\n\
    <p>\n";
	if (dir == NULL)
	{
		std::cerr << "Error: could not open the following path" << path << std::endl;
		return "";
	}
	if (Directory[0] != '/')
		Directory = "/" + Directory;
	for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir))
	{
		Autoindex_Page += geit(std::string(dirEntry->d_name), Directory, MainServer.host);
	}
	Autoindex_Page += "\
    </p>\n\
    </body>\n\
    </html>\n";
	closedir(dir);
	return Autoindex_Page;
}
