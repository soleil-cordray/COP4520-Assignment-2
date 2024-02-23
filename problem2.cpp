#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

std::mutex mtx;               // synchronization
std::condition_variable cond; // signaling
int currGuest = 1;            // guest #
int nGuests;                  // total guests

void visitShowroom(int id) {
    std::unique_lock<std::mutex> lock(mtx); // lock curr thread

    cond.wait(lock, [id] { return id == currGuest; });

    std::cout << "Guest " << id << " is visiting the showroom." << std::endl;

    // simulate visit duration
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Guest " << id << " has left the showroom." << std::endl;

    currGuest++;

    // ensure correct thread notified
    lock.unlock();
    cond.notify_all();
}

int main()
{
    std::cout << "Enter the total number of guests: ";
    std::cin >> nGuests;

    if (nGuests < 1)
    {
        std::cout << "Invalid number of guests." << std::endl;
        return 0;
    }

    // make thread for each guest
    std::vector<std::thread> guests;
    for (int i = 1; i <= nGuests; ++i)
    {
        guests.emplace_back(visitShowroom, i);
    }

    // wait for all threads to complete
    for (auto &guest : guests)
    {
        guest.join();
    }

    std::cout << "All guests have visited the showroom!" << std::endl;

    return 0;
}
