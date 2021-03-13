#include <fstream>
#include <string> 
#include <iostream>
#include <stdlib.h>
#include <list>
#include "Machine.h"

int main () {
	int nbr_machines = 0, i_money = 0, t_days = 0;
	//int curr_machine = 0, curr_money = 0, curr_day = 0;
	int D = 0, P = 0, R = 0, G = 0;
	int end_run = 0, case_nbr = 0, f_money = 0;
	std::list<Machine*> MachineList;

	std::ifstream input("input.txt");

	while (end_run == 0) {
		case_nbr++;
		input >> nbr_machines >> i_money >> t_days;
		if (nbr_machines == 0 && i_money == 0 && t_days == 0) {
			end_run = 1;
			std::cout << "End of file." << std::endl;
		}
		else if (nbr_machines != 0) {
			populate_dict(&input, &MachineList);
			f_money = optim_solver(nbr_machines, i_money, t_days, &MachineList);
			std::cout << "Case " << case_nbr << ": " << f_money << std::endl;
		}
		else std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
	}
}