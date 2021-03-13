#ifndef MACHINE
#define MACHINE

class Machine {
private:
	int avail, price, resell, gain;

public:
	Machine(int D = 0, int P = 0, int R = 0, int G = 0) :avail{ D }, price{ P }, resell{ R }, gain{ G } {};
	~Machine() {};
	int getAvailability() const { return avail; };
	int getPrice() const { return price; };
	int getResellPrice() const { return resell; };
	int getGain() const { return gain; };
};

#endif // !MACHINE

