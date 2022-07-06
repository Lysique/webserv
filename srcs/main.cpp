/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:57:05 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/06 09:53:23 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Server.hpp"
#include "config/ParserConfig.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "usage: " << argv[0] << " <config file>" << std::endl;
		return (1);
	}

    try 
    {
        ParserConfig    cf(argv[1]);
        std::vector<ServerMembers> servers = cf.getConfig();

		Server serv(servers);		
        serv.run();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
	return (0);
}
