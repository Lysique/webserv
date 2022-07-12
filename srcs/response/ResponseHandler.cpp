/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/12 11:31:43 by tamighi          ###   ########.fr       */
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
	}
	//	No index needed here
	else
		path = find_file_path(curr_loc.root + request.location);


	//	Change error_code and file_path with error page if needed
	error_code = check_error_code(path);
	if (error_code == 404 && curr_loc.autoindex == true)
	{
		// path = autoindex path
		path = find_file_path("/autoindex.html");
		error_code = 200;
	}
	if (error_code != 200)
		path = find_file_path(curr_loc.error_pages[error_code]);

	//	Retrieve the requested file !Check for autoindex (in error response)
	file = retrieve_file(path);

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
		if (path.substr(path.rfind('.')) == it->first)
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
		dup2(fd[1], 1);
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
		{
			std::cout << "Not allowed\n";
			return (405);
		}
	}
	else
	{
		std::cout << "Not implemented\n";
		return (501);
	}
	
	//	Check if we can access to path
	if (access(path.c_str(), F_OK) < 0)
	{
		std::cout << "Not found\n";
		return (404);
	}

	//	Check if read access
	if (access(path.c_str(), R_OK) < 0)
	{
		std::cout << "No read access\n";
		return (403);
	}

	//	Check if path is a file; if not : autoindex, else 404
	if (!is_file(path))
	{
		std::cout << "Not a file\n";
		return (404);
	}
	else
		std::cout << "Is a file\n";

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
		return ("application/octet-stream");
}
