/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/12 16:08:48 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(std::vector<ServerMembers> s)
	: servers(s)
{
	ErrorResponses[200] = "OK";
	ErrorResponses[204] = "No Content";
	ErrorResponses[403] = "Forbidden";
	ErrorResponses[404] = "Not Found";
	ErrorResponses[405] = "Method Not Allowed";
	ErrorResponses[413] = "Payload Too Large";
	ErrorResponses[501] = "Not Implemented";
}

ResponseHandler::~ResponseHandler()
{
}

std::string ResponseHandler::manage_request(RequestMembers r)
{
	ServerMembers	current_server;
	LocationMembers	current_location;
	request = r;

	//	Find correct server
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (request.port == servers[i].port)
			current_server = servers[i];
	}

	//	Find correct location in server
	//	Need to find based on precision and only the beginning should match!!
	for (size_t i = 0; i < current_server.locations.size(); ++i)
	{
		if (request.location.find(current_server.locations[i].uri) != std::string::npos)
		{
			if (current_location.uri < current_server.locations[i].uri)
				current_location = current_server.locations[i];
		}
	}
	this->curr_loc = current_location;
	return (write_response());
}

std::string	ResponseHandler::write_response(void)
{
	int			error_code;
	std::string	file;
	std::string	response;
	std::string	path;

	//	Find path of the requested file
	
	//	Try to find index if needed
	if (curr_loc.uri == request.location)
	{
		for (size_t i = 0; i < curr_loc.index.size(); ++i)
		{
			path = find_file_path(curr_loc.root + request.location + curr_loc.index[i]);
			if (is_file(path))
				break ;
		}
		path = find_file_path(curr_loc.root + request.location);
	}
	//	No index needed here
	else
		path = find_file_path(curr_loc.root + request.location);


	//	Change error_code and file_path with error page if needed
	error_code = check_error_code(path);
	if (error_code != 200)
		path = find_file_path(curr_loc.error_pages[error_code]);

	//	Retrieve the requested file
	if (is_file(path))
		file = retrieve_file(path);
	else
		file = get_autoindex(path);

	//	Check if Payload too large
	if (file.size() > curr_loc.max_body_size)
	{
		error_code = 413;
		path = find_file_path(curr_loc.error_pages[error_code]);
		file = retrieve_file(path);
	}

	//	Exec cgis
	for (std::map<std::string, std::string>::iterator it = curr_loc.cgis.begin();
			it != curr_loc.cgis.end(); ++it)
	{
		size_t	idx = path.rfind('.');
		if (idx != std::string::npos && path.substr(idx) == it->first)
			file = exec_cgi(path, it->second);
	}

	//	Headers
   	response += "HTTP/1.1 ";
	response += std::to_string(error_code);
	response += " ";
	response += ErrorResponses[error_code];
	response += "\nServer: serv_name";
	response += "\nDate: " + get_date();
	response += "\nContent-Length: " + std::to_string(file.size());
	response += "\nContent-Type: " + get_content_type(path);

	//	Body
	response += "\r\n\r\n";
	response += file;
	response += "\r\n\r\n";
	return (response);
}

std::string	ResponseHandler::exec_cgi(std::string file_path, std::string exec_path)
{
	std::vector<const char *>	env;
	std::vector<const char *>	exec;

	int							pid;
	int							fd[2];
	char 						buf[65535];

	//	Create execution command and environnment
	exec.push_back(exec_path.c_str());
	exec.push_back(file_path.c_str());
	exec.push_back(0);

	for (size_t i = 0; i < request.env.size(); ++i)
		env.push_back(request.env[i].c_str());
	env.push_back(0);

	//	Pipe and fork
	if (pipe(fd) == -1)
		throw std::runtime_error("Pipe failed.");
	pid = fork();
	if (pid == -1)
		throw std::runtime_error("Fork failed.");
	else if (pid == 0)
	{
		//	Execute command
		if (dup2(fd[1], 1) == -1)
			throw std::runtime_error("Dup2 failed.");
		execve(exec[0], (char **)&exec[0], (char **)&env[0]);
		exit(0);
	}
	else
	{
		//	Read the output and return

		bzero(buf, 65535);
		if (read(fd[0], buf, 65535) == -1)
			throw std::runtime_error("Read failed.");
		close(fd[0]);
		close(fd[1]);

		return (buf);
	}
}

int	ResponseHandler::check_error_code(std::string &path)
{
	//	Check if allowed method (405) or not implemented method (501)
	if (is_method_implemented())
	{
		if (!is_method_allowed())
			return (405);
	}
	else
		return (501);
	
	//	Check if we can access to path
	if (access(path.c_str(), F_OK) < 0)
		return (404);

	//	Check if read access
	if (access(path.c_str(), R_OK) < 0)
		return (403);

	//	Check if path is a file; if not : autoindex, else 404
	if (!is_file(path) && curr_loc.autoindex == false)
		return (404);

	return (200);
}

bool	ResponseHandler::is_method_allowed()
{
	for (size_t i = 0; i < curr_loc.allowedMethods.size(); ++i)
	{
		if (curr_loc.allowedMethods[i] == request.method)
			return (true);
	}
	return (false);
}

bool	ResponseHandler::is_method_implemented(void)
{
	if (request.method == "GET" || request.method == "POST" || request.method == "DELETE")
		return (true);
	return (false);
}
	
std::string	ResponseHandler::retrieve_file(std::string path)
{
    std::ostringstream	sstr;
	std::ifstream		ifs(path);

    sstr << ifs.rdbuf();
    return (sstr.str());
}

std::string	ResponseHandler::find_file_path(std::string path)
{
	char		cwd[256];

	if (getcwd(cwd, 256) == NULL)
		throw std::runtime_error("Getcwd failed.");
	if (path.find(cwd) != std::string::npos)
		return (path);
	return (cwd + path);
}

bool	ResponseHandler::is_file(std::string path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) < 0)
		throw std::runtime_error("Stat failed.");
	return (S_ISREG(s.st_mode));
}

std::string ResponseHandler::get_date(void)
{
	time_t		rawtime;
	struct tm	*timeinfo;
	char		buff[100];

	time(&rawtime);
 	timeinfo = localtime(&rawtime);
	strftime(buff, 100, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return (std::string(buff));
}

std::string	ResponseHandler::get_content_type(std::string file)
{
	std::string	extension;
	size_t		idx;
	
	idx = file.rfind('.');
	if (idx != std::string::npos)
		extension = file.substr(idx + 1);

	if (extension == "html" || extension == "php")
		return ("text/html");
	else if (extension == "xml")
		return ("text/xml");
	else if (extension == "js")
		return ("text/javascript");
	else if (extension == "css")
		return ("text/css");
	else if (extension == "cpp")
		return ("text/plain");

	else if (extension == "pdf")
		return ("application/pdf");
	else if (extension == "doc")
		return ("application/msword");

	else if (extension == "jpeg" || extension == "jpg" || extension == "pjp"
			|| extension == "jfif" || extension == "pjpeg")
		return ("image/jpeg");
	else if (extension == "png")
		return ("image/png");
	else if (extension == "gif")
		return ("image/gif");
	else if (extension == "ico")
		return ("image/x-icon");

	else if (extension == "mp4")
		return ("video/mp4");
	else if (extension == "webm")
		return ("video/webm");
	else if (extension == "mpeg")
		return ("video/mpeg");

	else if (extension == "mp3")
		return ("audio/mpeg");

	else
		return ("text/html");
}

std::string ResponseHandler::dir_to_html(std::string dir_entry, std::string path, std::string host)
{
	std::stringstream ss;
	if (dir_entry != ".." && dir_entry != ".")
		ss << "\t\t<p><a href=\"http://" + host + path + "/" + dir_entry + "\">" + dir_entry + "/" + "</a></p>\n";
	return ss.str();
}

std::string ResponseHandler::get_autoindex(std::string path)
{
	std::string	fullpath = path;
	DIR *dir = opendir(fullpath.c_str());

	std::string Autoindex_Page =
	"<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
    <title>" + path + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>INDEX</h1>\n\
    <p>\n";

	std::cout << path << std::endl;
	if (dir == NULL)
		throw std::runtime_error("Opendir failed : " + fullpath);
	
	if (path[0] != '/')
		path = "/" + path;

	for (struct dirent *dir_entry = readdir(dir); dir_entry; dir_entry = readdir(dir))
		Autoindex_Page += dir_to_html(std::string(dir_entry->d_name), path, request.host);

	Autoindex_Page += "\
    </p>\n\
    </body>\n\
    </html>\n";
	closedir(dir);
	return Autoindex_Page;
}
