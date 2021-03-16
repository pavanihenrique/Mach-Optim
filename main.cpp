/* ****************************************************************************
*
* Henrique Pavani Pereira Ramos
* Étudiant parcours ingénierie - Télécom Paris
* henrique.ramos@telecom-paris.fr
* Palaiseau, France
*
* Optimization problem where the objective is to maximize revenue during a
*   restructuring period of a company by choosing which machines to buy. The
*   problem description can be found on the Problem_Description.pdf available
*   on github alongside with code files.
* 
* Before use: replace the input file name at main().
*
*/

#include <fstream>
#include <string> 
#include <iostream>
#include <stdlib.h>
#include <list>
#include <map>
#include <memory>
#include "Machine.h"

// using shared pointers for auto destruction of machine objects when the map is cleared.
using MachineMap = std::map<unsigned long long, std::shared_ptr<Machine>>;
using MachineList = std::list<std::shared_ptr<Machine>>;

void populate_dict(std::ifstream& input, MachineMap& dict, 
					unsigned long long N) {
/* populate_dict(std::ifstream& input, MachineMap& dict,
*					unsigned long long N, unsigned long long T)
* 
* Insert the N machines information into a map, in order of availability.
* 
* Input parameters:
*   - std::ifstream& input: input file;
*   - MachineMap& dict: machine map;
*   - unsigned long long N: total number of machines.
* 
* No output.
* 
*/
	unsigned long long D = 0, P = 0, R = 0, G = 0;
	unsigned long long i = 0, key = 1;
	MachineList aux_list;
	
	// lists allow sorting, but map methods are better suited for this solution.
	for (i = 0; i < N; i++) {
		input >> D >> P >> R >> G;
		std::shared_ptr<Machine> p(new Machine(D, P, R, G));
		aux_list.push_back(p);
	}

	aux_list.sort([](const std::shared_ptr<Machine> f, const std::shared_ptr<Machine> s) { return f->getAvailability() < s->getAvailability(); });
	
	for (MachineList::iterator it = aux_list.begin(); it != aux_list.end();) {
		MachineList::iterator it2 = it;
		++it2;
		std::shared_ptr<Machine> p2(*it);
		dict.insert(make_pair(key, p2));
		++key;
		aux_list.erase(it);
		it = it2;
		if (it == aux_list.end()) break;
	}

	return;
}

void reset_dict(MachineMap& dict) {
/* reset_dict(MachineMap& dict)
*
* Clears the map before processing the next case.
*
* Input parameters:
*   - MachineMap& dict: machine map;
*
* No output.
*
*/
	
	// reseting shared poiters will auto destruct objects since there's no
	//   other pointers to those.
	for (MachineMap::iterator it = dict.begin(); it != dict.end(); ++it) {
		(it->second).reset();
	}
	MachineMap::iterator dict_begin = dict.begin();
	MachineMap::iterator dict_end = dict.end();
	dict.erase(dict_begin, dict_end);
}

unsigned long long buy_decision(MachineMap& dict,
								unsigned long long curr_machine,
								unsigned long long curr_money,
								unsigned long long curr_day,
								unsigned long long T, unsigned long long i_index) {
/* unsigned long long buy_decision(MachineMap& dict,
*								unsigned long long curr_machine,
*								unsigned long long curr_money,
*								unsigned long long curr_day,
*								unsigned long long T, int i_index) {
*
* Takes the decision which machine available that day to buy (or not buy any).
*
* Input parameters:
*   - MachineMap& dict: machine map;
*   - unsigned long long curr_machine: current machine in use;
*   - unsigned long long curr_money: current amount of money available;
*   - unsigned long long curr_day: current day;
*   - unsigned long long T: total number of days on the restructuring period;
*   - int i_index: map key to the next unprocessed machine;
*
* Output: 0 if the decision is to not buy any machine, otherwise the result is
*           the key map to the machine to be bought.
*
*/
	unsigned long long result = 0, curr_projection = 0, breakeven = 0;
	
	unsigned long long days_left = T - curr_day;
	unsigned long long curr_result = curr_money;
	unsigned long long avail_money = curr_money;

	unsigned long long best_candidate = curr_machine;
	unsigned long long best_result = curr_result;
	unsigned long long best_breakeven = curr_day;

	long long a = 0, b = 0, q = 0;
	int sign = 0;

	// avail_money represents the available amount of money if the current machine is sold
	if (curr_machine != 0) {
		curr_result = curr_result + dict[curr_machine]->getGain() * (days_left + 1) + dict[curr_machine]->getResellPrice();
		avail_money = avail_money + dict[curr_machine]->getResellPrice();
	}

	best_result = curr_result;

	for (MachineMap::iterator it = dict.find(i_index); it != dict.end(); ++it) {
		if ((*(it->second)).getAvailability() == curr_day) {
			if (avail_money >= (*(it->second)).getPrice()) {
				// the projected result if this machine is bought and operated until the end
				result = avail_money - (*(it->second)).getPrice() + (*(it->second)).getGain() * days_left + (*(it->second)).getResellPrice();

				// breakeven represents the day that the new machine's operation will overcome the previous projection
				breakeven = curr_day + 1;

				// d_t = (P2 - R1)/(G2 - G1)
				a = (*(it->second)).getPrice();
				b = (*(it->second)).getGain();
				if (curr_machine != 0) {
					a = a - dict[curr_machine]->getResellPrice();
					b = b - dict[curr_machine]->getGain();
				}
				if (b == 0) continue;
				if (a < 0 || b < 0) {
					if (a < 0 && b < 0) {
						a = -a;
						b = -b;
					}
					else if (a < 0) {
						a = -a;
						sign = 1;
					}
					else {
						b = -b;
						sign = 1;
					}
				}
				q = a / b + (a % b != 0);
				if (sign) q = -q;
				breakeven = breakeven + q;

				if (result > best_result || (best_candidate != curr_machine && result == best_result && breakeven < best_breakeven)) {
					best_candidate = it->first;
					best_result = result;
					best_breakeven = breakeven;
				}
			}
		}
		else if ((*(it->second)).getAvailability() > curr_day) break; // no more machines available on the current day.
	}

	// search in the breakeven period of the best candidate if there's any other machine that would present a bettter result
	//   machines available outside of this period will not influence the decision of buying or not the machine being evaluated.
	if (best_candidate != curr_machine && best_breakeven > curr_day + 2) {
		curr_projection = curr_money;
		avail_money = curr_money;
		for (MachineMap::iterator it = dict.find(i_index); it != dict.end(); ++it) {
			if (curr_machine != 0) {
				curr_projection = curr_projection + dict[curr_machine]->getGain() * ((*(it->second)).getAvailability() - curr_day);
				avail_money = curr_projection + dict[curr_machine]->getResellPrice();
			}
			if ((*(it->second)).getAvailability() >= best_breakeven) break;
			if (avail_money >= (*(it->second)).getPrice()) {
				days_left = T - (*(it->second)).getAvailability();
				result = avail_money - (*(it->second)).getPrice();
				result = result + (*(it->second)).getGain() * days_left;
				result = result + (*(it->second)).getResellPrice();
				if (result > best_result) return 0;
			}
			
		}
	}

	if (best_candidate == curr_machine) return 0;
	
	return best_candidate;
}

unsigned long long optim_solver(unsigned long long N, unsigned long long M,
								unsigned long long T, MachineMap& dict) {
/* unsigned long long optim_solver(unsigned long long N, unsigned long long M,
*								unsigned long long T, MachineMap& dict) {
*
* Solver implemented taking into account the breakeven period when deciding
*   whether or not to buy a machine, not only its immediate benefits.
*
* Input parameters:
*   - unsigned long long N: total number of machines;
*   - unsigned long long M: initial amount of money available;
*   - unsigned long long T: total number of days on the restructuring period;
*   - MachineMap& dict: machine map.
*
* Output: amount of money at the end of the restructuring period.
*
*/
	unsigned long long curr_money = M, curr_day = 1, next_dec = 0;
	unsigned long long curr_machine = 0, buy = 0;

	MachineMap::iterator it = dict.begin();
	while (it != dict.end()) {
		next_dec = (it->second)->getAvailability();
		if (curr_machine != 0) curr_money = curr_money + (dict[curr_machine]->getGain()) * (next_dec - curr_day - 1); // the machine is operated at least until the next decision
		curr_day = next_dec;
		buy = buy_decision(dict, curr_machine, curr_money, curr_day, T, it->first); // 0 to not buy any machine, map key instead
		if (buy) {
			// the machine is not operated the same day it is bought
			if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getResellPrice();
			curr_machine = buy;
			if (curr_machine != 0) curr_money = curr_money - dict[curr_machine]->getPrice();
		}
		else if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getGain(); // machine operation
		while ((it->second)->getAvailability() == curr_day) {
			// avance to the first machine available on a later day
			++it;
			if (it == dict.end()) break;
		}
	}

	// machine operation and reselling after the end of the restructuring period
	if (curr_machine != 0) {
		curr_money = curr_money + (dict[curr_machine]->getGain()) * (T - curr_day);
		curr_money = curr_money + dict[curr_machine]->getResellPrice();
	}

	return curr_money;
}

int main () {
	int nbr_machines = 0;
	unsigned long long i_money = 0, t_days = 0;
	int end_run = 0, case_nbr = 0;
	unsigned long long f_money = 0;
	MachineMap MachineDict;

	std::ifstream input("input.txt");	// replace input file name

	while (end_run == 0) {
		case_nbr++;
		input >> nbr_machines >> i_money >> t_days;
		if (nbr_machines == 0 && i_money == 0 && t_days == 0) {
			// line with 0 0 0 indicates the end of file
			end_run = 1;
			std::cout << "End of file." << std::endl;
		}
		else if (i_money == 0 || t_days <= 1) {
			// there's no viable solution that buys any machine
			std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
		}
		else if (nbr_machines != 0) {
			populate_dict(input, MachineDict, nbr_machines);	// MachineDict is ordenated by machine availability
			f_money = optim_solver(nbr_machines, i_money, t_days, MachineDict);		// final result
			std::cout << "Case " << case_nbr << ": " << f_money << std::endl;
			reset_dict(MachineDict);	// clears the machine map in order to process the next case
		}
		else {
			// there's no machines available to be bought
			std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
		}
	}
}