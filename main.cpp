#include "DSSimul.h"
//#include <unistd.h>
#include <assert.h>
#include <sstream>

void start_election(Process *dp, set<int> neibs) {
	bool sent = false;
	for (auto n: neibs)
		if (n > dp->node) {
	   		dp->networkLayer->send(dp->node, n, Message("BULLY_ELECTIONS"));
	   		printf("[%d]:Elections sent to [%d]\n", dp->node, n);
	   		dp->context_bully.waiting_ping = true;
			dp->context_bully.start_time = dp->networkLayer->tick;
			sent = true;
		}
	if (!sent)
		for (auto n: neibs) {
	   		dp->networkLayer->send(dp->node, n, Message("BULLY_COORDINATOR"));
	   		printf("[%d]:Coordinator sent to [%d]\n", dp->node, n);
	}
}

int workFunction_BULLY(Process *dp, Message m) {
	string s = m.getString();
    NetworkLayer *nl = dp->networkLayer;
    if (!dp->isMyMessage("BULLY", s)) return false;
    set<int> neibs = dp->neibs();
	if (s == "BULLY_ELECTIONS_INIT" || s == "BULLY_ELECTIONS") {
		if (s == "BULLY_ELECTIONS") {
			// TODO: Check Visanty's n > dp->node
			printf("[%d]: Get ELECTIONS from [%d]\n", dp->node, m.from);
			nl->send(dp->node, m.from, Message("BULLY_ALIVE"));
		}
		start_election(dp, neibs);
	}
	if (s == "BULLY_COORDINATOR") {
		printf("[%d]: Get COORDINATOR from [%d]\n", dp->node, m.from);
		if (dp->node > m.from)
			start_election(dp, neibs);
		else
			dp->context_bully.waiting_coordinator = false;
	}
	if (s == "BULLY_ALIVE") {
		printf("[%d]: Get ALIVE from [%d]\n", dp->node, m.from);
		dp->context_bully.waiting_ping = false;
		dp->context_bully.waiting_coordinator = true;
		dp->context_bully.start_time = nl->tick;
	}
	if (s == "*TIME") {
		printf("[%d]:Tick\n", dp->node);
		if (dp->context_bully.waiting_ping && nl->tick > dp->context_bully.start_time + dp->context_bully.delta)
			for (auto n : neibs)
				nl->send(dp->node, n, Message("BULLY_COORDINATOR"));
		if (dp->context_bully.waiting_coordinator && nl->tick > dp->context_bully.start_time + dp->context_bully.delta)
			start_election(dp, neibs);
	}
	return true;
}


int workFunction_TEST(Process *dp, Message m)
{
    string s = m.getString();
    NetworkLayer *nl = dp->networkLayer;
    if (!dp->isMyMessage("TEST", s)) return false;
    set<int> neibs = dp->neibs(); 
    if (s == "TEST_HELLO") {
        int val = m.getInt();
        printf("TEST[%d]: HELLO %d message received from %d\n", dp->node, val, m.from);
        // Рассылаем сообщение соседям
        if (val < 2) {
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("TEST_HELLO", val+1));
            }
        } else {
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("TEST_BYE"));
            }
        }
    } else if (s == "TEST_BYE") {
        printf("TEST[%d]: BYE message received from %d\n", dp->node, m.from);
    }
    return true;
}

int main(int argc, char **argv)
{
    string configFile = argc > 1 ? argv[1] : "config.data";
    World w; 
    w.registerWorkFunction("BULLY", workFunction_BULLY);
    if (w.parseConfig(configFile)) {
        this_thread::sleep_for(chrono::milliseconds(3000000));
	} else {
        printf("can't open file '%s'\n", configFile.c_str());
    }
}

