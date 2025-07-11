#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
using namespace std;
const int max_teller = 10;
const int max_transaction = 5;
const int max_customers = 135;         //per day (5 working days per week) 35000/365-(2*52) = 135
const int work_time_minutes = 480;     //8 AM - 4 PM 


string transaction_names[max_transaction];
bool tellerSkills[max_teller][max_transaction] = { false };   //start no skills
int avgDurations[max_transaction];
int numTransactions = 0;
int numTellers = 0;
int tellerBusyUntil[max_teller] = { 0 };            //the tellers start free
int customers[max_customers * 6];                   //multipled 6 cuz the array of customer data contains 6 elements
int breakStart[max_teller];                         //to calculate the break time
int breakEnd[max_teller];


void setupProgram();
void simulation();
void generateReports(int customer_count);
int getRandomDuration(int avg);
int assignTeller(int transType, int currentTime, int duration);


int main() {
    srand(time(0));
    setupProgram();
    simulation();
    return 0;
}


void setupProgram() {
    do {
        cout << "Enter number of tellers (1-" << max_teller << "): ";
        cin >> numTellers;
    } while (numTellers < 1 || numTellers > max_teller);

    int totalDuration = 0;
    cout << "Enter transaction names (do not use special characters or spaces):\n";

    for (int i = 0; i < max_transaction; i++) {
        cout << "Enter transaction name (type 'none' to stop): ";
        string name;//to access break condition
        cin >> name;
        if (name == "none")  
            break;

        transaction_names[numTransactions] = name;

        cout << "Enter average duration (minutes): ";
        cin >> avgDurations[numTransactions];

        totalDuration += avgDurations[numTransactions];
        numTransactions++;
    }
    for (int i = 0; i < numTellers; i++) {
        for (int j = 0; j < numTransactions; j++) {
            if (totalDuration > 100 && numTransactions == 5) {
                tellerSkills[i][j] = true;  // all tellers are fully skilled because the day is busy
            }
            else {
                tellerSkills[i][j] = rand() % 2;  // randomly assign skill: 0 = false, 1 = true
            }
        }
        

        breakStart[i] = 240 + rand() % 60;               // breaks start at 12PM (minute 240)
        breakEnd[i] = breakStart[i] + 5 + (rand() % 11); // min break 5 mins, max 15 mins
    }
}


int assignTeller(int transType, int currentTime, int duration) {//current time is arrival time 

    int chosenTeller = -1;
    int earliestStart = INT_MAX;

    for (int i = 0; i < numTellers; i++) {
        
        if (!tellerSkills[i][transType]) continue;// Skip if teller doesn't know the transaction

        
        int freeAt = tellerBusyUntil[i];// Calculate when this teller will actually be free

        // Adjust free time if it's during break
        if (freeAt >= breakStart[i] && freeAt < breakEnd[i]) {//if the teller become free during break time make the actual free time at the end of the break
            freeAt = breakEnd[i];
        }

        // Adjust currentTime if teller is on break now
        int availableFrom = max(currentTime, freeAt);
        if (availableFrom >= breakStart[i] && availableFrom < breakEnd[i]) {
            availableFrom = breakEnd[i];
        }

        
        if (availableFrom + duration > work_time_minutes) continue;// the teller serve the customer before the bank closes
        if (availableFrom < earliestStart) {   //first teller become free

            earliestStart = availableFrom;
            chosenTeller = i;
        }
    }

    return chosenTeller;
}

void simulation() {
    int customer_count = rand() % max_customers;
    cout << "    DAY SIMULATION " << endl;
    cout << "Customers today: " << customer_count << endl;

    for (int i = 0; i < numTellers; i++) {
        tellerBusyUntil[i] = 0;
    }

    for (int i = 0; i < customer_count; i++) {
        int id = 1 + i;

        int raw = rand() % 100;
        int arrival_time;
        if (raw < 80)  // to make the bigger percent comes in first hours 
            arrival_time = 1+(rand() % 360);
        else
            arrival_time =1+ (360 + rand() % 120);  //to make the remain comes in last hour 

        int transType = rand() % numTransactions;
        int duration = getRandomDuration(avgDurations[transType]);
        int tellerId = assignTeller(transType, arrival_time,duration);
        int startTime = max(arrival_time, tellerBusyUntil[tellerId]);
        if (tellerId == -1||(arrival_time >= 420 && duration > 15) ){              //if the teller dont have the skill
            customers[i * 6 + 0] = id;     //multipled 6 cuz the array of customer data contains 6 elements like line #19
            customers[i * 6 + 1] = arrival_time;
            customers[i * 6 + 2] = 0;
            customers[i * 6 + 3] = 0;
            customers[i * 6 + 4] = 0;
            customers[i * 6 + 5] = 0;
            continue;
        }
        tellerBusyUntil[tellerId] = startTime + duration; //calculate teller utilization

        customers[i * 6 + 0] = id;
        customers[i * 6 + 1] = arrival_time;
        customers[i * 6 + 2] = transType;
        customers[i * 6 + 3] = duration;
        customers[i * 6 + 4] = 1;  
        customers[i * 6 + 5] = startTime;
    }

    generateReports(customer_count);

}


int getRandomDuration(int avg) {    //to make the duration average and to make it realistic we make the cin in setup function 
    int variation = avg * 0.2;
    return max(1, avg - variation + rand() % (2 * variation + 1));
}
void generateReports(int customer_count) {
    cout << "\n         CUSTOMER REPORT\n";
    cout << "ID" << setw(10) << "Arrival " << setw(15) << " Transaction Type"
        << setw(10) << "Duration" << setw(10) << "Start" << setw(9) << "  End" << setw(10) << " wait time"<<setw(10) << " Served\n";

    for (int i = 0; i < customer_count; i++) {
        int id = customers[i * 6 + 0];
        int arrival_time = customers[i * 6 + 1];
        int transType = customers[i * 6 + 2];
        int duration = customers[i * 6 + 3];
        int served = customers[i * 6 + 4];
        int startTime = customers[i * 6 + 5];
        if (served == 1) {
            int end_time = startTime + duration;
            int wait_time = startTime - arrival_time;
            cout << id << setw(10) << arrival_time
                << setw(15) << transaction_names[transType]
                << setw(10) << duration << setw(12)
                    << startTime << setw(10) << end_time << setw(10) << wait_time << setw(10) << "Yes\n";
        }
        else
        {
            cout << id << setw(10) << arrival_time
                << setw(15) << transaction_names[transType]
                << setw(10) << "-" << setw(12)
                    << "-" << setw(10) << "-" << setw(10) << "-" << setw(10) << "no\n";
        }
    }
    cout << "\n         TELLER UTILIZATION\n";
    for (int i = 0; i < numTellers; i++) {
        float utilization = static_cast<float>(tellerBusyUntil[i]) / work_time_minutes * 100;
        cout << "Teller " << i + 1 << ": "
            << fixed << setprecision(1) << utilization << "% busy\n";
    }
    cout << "\n         TELLER BREAKS\n";
    for (int i = 0; i < numTellers; i++) { 
        int breaktime = breakEnd[i] - breakStart[i];
        cout << "Teller " << i + 1 << ": Break from " << breakStart[i]
            << " to " << breakEnd[i] << " total break time:  "<< breaktime<<"\n";
    }
}