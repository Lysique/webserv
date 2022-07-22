/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tamighi <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/07 10:42:46 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/22 17:26:17 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseHandler.hpp"

ResponseHandler::ResponseHandler(std::vector<ServerMembers> s)
	: servers(s)
{
	ErrorResponses[200] = "OK";
	ErrorResponses[403] = "Forbidden";
	ErrorResponses[404] = "Not Found";
	ErrorResponses[405] = "Method Not Allowed";
	ErrorResponses[413] = "Payload Too Large";
	ErrorResponses[501] = "Not Implemented";
	ErrorResponses[502] = "Bad Gateway";
}

ResponseHandler::~ResponseHandler()
{
}

void ResponseHandler::manage_request(int socket, RequestMembers r)
{
	request = r;
	curr_sock = socket;

	http_responses[socket] = manage_response();
	std::cout << http_responses[socket] << "xx" << std::endl;
	write_response();
}

std::string	ResponseHandler::manage_response(void)
{
	int			error_code;
	std::string	path;
	std::string	file;

	//	There is still data from previous write
	if (http_responses[curr_sock] != "")
		return (http_responses[curr_sock]);

	//	The client is not done sending data
	if (request.parsed == false)
		return ("HTTP/1.1 100 Continue\r\n\r\n");

	get_current_loc();

	//	Check if correct method
	error_code = check_method();
	if (error_code != 200)
		return (make_response(file, error_code, path));

	//	Add index
	if (curr_loc.uri == request.location && curr_loc.autoindex == false)
		request.location += curr_loc.index;

	//	Check if correct path
	path = get_path(curr_loc.root + request.location);
	error_code = check_path_access(path);
	if (error_code != 200)
		return (make_response(file, error_code, path));

	//	Manage requests
	if (request.method == "DELETE")
		remove(path.c_str());
	else if (request.method == "GET")
	{
		if (is_file(path))
			file = retrieve_file(path);
		else
			file = get_autoindex(path, request.location);
	}
	else if (request.method == "POST" && is_file(path))
		file = retrieve_file(path);

	//	Exec cgis
	for (std::map<std::string, std::string>::iterator it = curr_loc.cgis.begin();
			it != curr_loc.cgis.end(); ++it)
	{
		size_t	idx = path.rfind('.');
		if (idx != std::string::npos && path.substr(idx) == it->first && is_file(path))
			file = exec_cgi(path, it->second);
	}

	//	Check body size
	if (file.size() > curr_loc.max_body_size)
		error_code = 413;

	//	Write response
	return (make_response(file, error_code, path));
}

std::string	ResponseHandler::make_response(std::string file, int error_code, std::string path)
{
	std::string	response;

	//	Get error page
	if (error_code != 200)
	{
		path = get_path(curr_loc.error_pages[error_code]);
		if (check_path_access(path) == 200)
			file = retrieve_file(path);
		else
			file = "";
		if (file.size() > curr_loc.max_body_size)
			file = "";
	}

	//	Headers
   	response += request.protocol;
	response += " ";
	response += std::to_string(error_code);
	response += " ";
	response += ErrorResponses[error_code];

	response += "\nDate: " + get_date();
	response += "\nContent-Length: " + std::to_string(file.size());
	response += "\nContent-Type: " + get_content_type(path);
	response += "\r\n\r\n";

	//	Body
	if (file != "")
	{
		response += file;
		response += "\r\n\r\n";
	}
	return (response);
}

void	ResponseHandler::get_current_loc(void)
{
	ServerMembers	current_server;

	//	Find correct server
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (request.port == servers[i].port)
		{
			current_server = servers[i];
			break ;
		}
	}

	//	Find correct location in server
	for (size_t i = 0; i < current_server.locations.size(); ++i)
	{
		if (request.location.find(current_server.locations[i].uri) == 0)
		{
			if (curr_loc.uri < current_server.locations[i].uri)
				curr_loc = current_server.locations[i];
		}
	}
}

void	ResponseHandler::write_response(void)
{
	int			ret;
	std::string	&buffer = http_responses[curr_sock];

	ret = write(curr_sock, buffer.c_str(), buffer.size());
	if (ret == -1)
		throw std::runtime_error("Write failed.");

	// Client disconnected
	if (ret == 0)
	{
		//close_connection(socket);
		return ;
	}

	buffer = buffer.substr(ret);
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

	for (std::map<std::string, std::string>::iterator it = request.postvals.begin();
			it != request.postvals.end(); ++it)
	{
		env.push_back((it->first + "=" + it->second).c_str());
	}
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
		//	Print bad gateway file
		std::string path = get_path(curr_loc.error_pages[502]);
		std::cout << retrieve_file(path);
		exit(0);
	}
	else
	{
		//	Read the output and return
		wait(0);
		bzero(buf, 65535);
		if (read(fd[0], buf, 65535) == -1)
			throw std::runtime_error("Read failed.");
		close(fd[0]);
		close(fd[1]);

		return (buf);
	}
}

int	ResponseHandler::check_method(void)
{
	if (!is_method_implemented())
		return (501);
	if (!is_method_allowed())
		return (405);
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
	
int	ResponseHandler::check_path_access(std::string path)
{
	//	Check if we can access to path
	if (access(path.c_str(), F_OK) < 0)
		return (404);

	//	Check if path is a file or if folder is allowed
	if (!is_file(path) && (curr_loc.autoindex == false || request.method != "GET"))
		return (404);

	//	Check if read access
	if (access(path.c_str(), R_OK) < 0)
		return (403);

	return (200);
}

std::string	ResponseHandler::get_path(std::string path)
{
	char		cwd[256];

	if (getcwd(cwd, 256) == NULL)
		throw std::runtime_error("Getcwd failed.");
	return (cwd + path);
}

std::string	ResponseHandler::retrieve_file(std::string path)
{
    std::ostringstream	sstr;
	std::ifstream		ifs(path);

    sstr << ifs.rdbuf();
    return (sstr.str());
}

bool	ResponseHandler::is_file(std::string path)
{
	struct stat	s;

	if (access(path.c_str(), F_OK) < 0)
		return (false);
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
	else if (extension == "cpp" || extension == "hpp")
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

std::string ResponseHandler::dir_to_html(std::string dir_entry, std::string path)
{
	std::stringstream ss;
	path = path.substr(path.find(curr_loc.root) + curr_loc.root.size());
	if (dir_entry != ".." && dir_entry != ".")
		ss << "\t\t<p><a href=\"http://" + request.host + ":" << request.port << path + dir_entry + "\">" + dir_entry + "/" + "</a></p>\n";
	return ss.str();
}

std::string ResponseHandler::get_autoindex(std::string fullpath, std::string path)
{
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

	if (dir == NULL)
		throw std::runtime_error("Opendir failed : " + fullpath);
	
	if (path.back() != '/')
		path += "/";

	for (struct dirent *dir_entry = readdir(dir); dir_entry; dir_entry = readdir(dir))
		Autoindex_Page += dir_to_html(std::string(dir_entry->d_name), path);

	Autoindex_Page += "\
    </p>\n\
    </body>\n\
    </html>\n";
	closedir(dir);
	return Autoindex_Page;
}
