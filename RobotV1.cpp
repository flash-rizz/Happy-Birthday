#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <random>
#include <ctime>
#include <iomanip>
#include "Battlefield.h"



using namespace std;

class Battlefield;

class Robot {
protected:
    int positionR;
    int positionC;
    string type = "";
    string name = "";
    int lives = 3;
    int kills = 0;
    int ammo = 10; // Each robot starts with 10 ammo shells
    string upgradeType = ""; // Attribute to store the current upgrade type

public:
    Robot(int r = 0, int c = 0, string robotType = "Robot", string robotName = "")
        : positionR(r), positionC(c), type(robotType), name(robotName) {
        name = string(robotName); // Ensure memory is allocated for each robot's name
    }

    virtual ~Robot() {}

    int r() const { return positionR; }
    int c() const { return positionC; }
    void setR(int r) { positionR = r; }
    void setC(int c) { positionC = c; }
    string robotType() const { return type; }
    string robotName() const { return name; }
    int robotLives() const { return lives; }
    int robotKills() const { return kills; }
    void setRobotLives(int live) { lives = live; }
    void setRobotKills(int kill) { kills = kill; }
    int getAmmo() const { return ammo; }
    void setAmmo(int ammoCount) { ammo = ammoCount; }
    string getUpgradeType() const { return upgradeType; }
    void setUpgradeType(const string& type) { upgradeType = type; }
    

    bool consumeAmmo() {
        if (ammo > 0) {
            ammo--;
            return true; // Ammo successfully consumed
        }
        cout << type << " " << name << " has no ammo left!" << endl;
        if (lives > 0) {
            lives--; // Deduct a life when ammo is 0
            cout << type << " " << name << " loses a life due to no ammo. Remaining lives: " << lives << endl;
            if (lives == 0) {
                cout << type << " " << name << " is destroyed due to no ammo and no lives left." << endl;
            }
        }
        return false; // No ammo left
    }

    void lostLive() {
        if (lives > 0) lives--;
    }
    virtual void robotRespawn(int r, int c) {
        setR(r);
        setC(c);
    }

    virtual void setLocation(int r, int c) {}
    virtual void action(Battlefield& bf) = 0;

    // Overloading the << operator
    friend ostream& operator<<(ostream& os, const Robot& robot) {
        os << "Robot Type: " << robot.robotType() << ", "
           << "Name: " << robot.robotName() << ", "
           << "Position: (" << robot.r() << "," << robot.c() << "), "
           << "Lives: " << robot.robotLives() << ", "
           << "Kills: " << robot.robotKills() << ", "
           << "Ammo: " << robot.getAmmo();
        return os;
    }
};

class ThinkingRobot : virtual public Robot {
    public:
        int upgTally = 0;
        vector<int> v;

        void upgradeTree();

        void robotRespawn(int r, int c) override {
            Robot::robotRespawn(r, c); // Call base method to set new position
            upgTally = 0;
            v.clear(); // Clear the upgrade path history
}
    };

class MovingRobot : virtual public Robot {
    public:
        MovingRobot(int r, int c, string robotType, string robotName)
            : Robot(r, c, robotType, robotName) {}


        virtual void move(Battlefield& bf);
    };

class ShootingRobot : virtual public Robot, public ThinkingRobot {
    protected:
        int range;

    public:
        ShootingRobot(int r, int c, string robotType, string robotName, int fireRange)
            : Robot(r, c, robotType, robotName), range(fireRange) {}

        virtual void fire(Battlefield& bf, const string& fireMethod);
    };

class SeeingRobot : virtual public Robot {
    public:
        SeeingRobot(int r, int c, string robotType, string robotName)
            : Robot(r, c, robotType, robotName) {}

        virtual void look(Battlefield& bf);
    };

class JumpBot : public MovingRobot {
    private:
        bool hasJumped = false; // Track if the bot has jumped this round

    public:
        JumpBot(int r, int c, string robotType, string robotName)
            : Robot(r, c, robotType, robotName), MovingRobot(r, c, robotType, robotName) {}

        void jump(Battlefield& bf) {
            // Decide whether to use the jump ability (50% chance)
            if (rand() % 2 == 0) {
                // Find new random coordinates that are not occupied
                int newR, newC;
                bool validPosition = false;

                while (!validPosition) {
                    newR = rand() % bf.get_NUM_OF_ROWS();
                    newC = rand() % bf.get_NUM_OF_COLS();

                    // Check if the new position is occupied
                    validPosition = true;
                    RobotNode* current = bf.getHead();
                    while (current) {
                        if (current->robot->r() == newR && current->robot->c() == newC) {
                            validPosition = false;
                            break;
                        }
                        current = current->next;
                    }
                }

                // Assign new coordinates
                setR(newR);
                setC(newC);
                hasJumped = true;

                cout << type << " " << name << " used JumpBot ability and jumped to (" << newC << "," << newR << ")." << endl;
            } else {
                cout << type << " " << name << " decided not to use JumpBot ability this round." << endl;
            }
        }

    void action(Battlefield& bf) override {
        hasJumped = false; // Reset jump state for this round

        if (getUpgradeType() == "jumpbot") {
            jump(bf); // Attempt to use the JumpBot ability
        }

        if (!hasJumped) {
            // Perform normal actions if the bot did not jump
            move(bf);
        }
    }
};
class HideBot {
    protected:
        bool isCurrentlyHidden = false;
        int remainingHides = 3;
    
    public:
        HideBot() {}
    
        void tryToHide() {
            if (remainingHides <= 0) {
                isCurrentlyHidden = false;
                return;
            }
    
            bool willHide = (std::rand() % 2 == 0); // 50% chance
    
            if (willHide) {
                isCurrentlyHidden = true;
                remainingHides--;
            } else {
                isCurrentlyHidden = false;
            }
        }
    
        bool isHidden() const {
            return isCurrentlyHidden;
        }
    
        int getRemainingHides() const {
            return remainingHides;
        }
    
        void endTurn() {
            isCurrentlyHidden = false;
        }
    };
    
class GenericRobot : public MovingRobot, public ShootingRobot, public SeeingRobot {
    public:
        GenericRobot(int r, int c, string robotType, string robotName)
            : Robot(r, c, robotType, robotName), MovingRobot(r, c, robotType, robotName), ShootingRobot(r, c, robotType, robotName, 10),
            SeeingRobot(r, c, robotType, robotName) {}

        void action(Battlefield& bf) override {
            if (getUpgradeType() == "jumpbot") {
                // Use JumpBot functionality
                JumpBot jumpBot(r(), c(), robotType(), robotName());
                jumpBot.jump(bf);
                return; // Skip the rest of the actions for this round
            }

            // Perform normal actions
            move(bf);
            look(bf);

            if (getUpgradeType() == "longshotbot") {
                fire(bf, "longshotbot");
            } else if (getUpgradeType() == "semiautobot") {
                fire(bf, "semiautobot");
            } else {
                fire(bf, "genericrobot");
            }
        }
    };



class BomberBot : public MovingRobot, public SeeingRobot {
public:
    BomberBot(int r, int c, string robotType, string robotName)
        : Robot(r, c, robotType, robotName), SeeingRobot(r, c, robotType, robotName), MovingRobot(r, c, robotType, robotName) {}

    void throwBomb(Battlefield& bf);

    void action(Battlefield& bf) override {
        move(bf);
        if (consumeAmmo()) { // Consume ammo before throwing a bomb
            throwBomb(bf);
        }
    }
};

class AngelBot : public SeeingRobot, public MovingRobot, public ShootingRobot {
public:
    AngelBot(int r, int c, string robotType, string robotName)
        : Robot(r, c, robotType, robotName), SeeingRobot(r, c, robotType, robotName), MovingRobot(r, c, robotType, robotName),
            ShootingRobot(r, c, robotType, robotName, 10) {
        lives = 5;
    }

    void action(Battlefield& bf) override {
        move(bf);
        look(bf);
        fire(bf, "genericrobot");
    }
};


class RobotNode {
    public:
        Robot* robot;
        RobotNode* next;

        RobotNode(Robot* robot) : robot(robot), next(nullptr) {}
    };

    class CustomQueue {
    private:
        RobotNode* front;
        RobotNode* rear;

    public:
        CustomQueue() : front(nullptr), rear(nullptr) {}

        void enqueue(Robot* robot) {
            RobotNode* newNode = new RobotNode(robot);
            if (rear) {
                rear->next = newNode;
            } else {
                front = newNode;
            }
            rear = newNode;
        }

        Robot* dequeue() {
            if (front == nullptr) return nullptr;
            RobotNode* temp = front;
            Robot* robot = temp->robot;
            front = front->next;
            if (front == nullptr) rear = nullptr;
            delete temp;
            return robot;
        }

        bool isEmpty() const {
            return front == nullptr;
        }
    };

class Battlefield {
    private:
        int NUM_OF_COLS;
        int NUM_OF_ROWS;
        int turns;
        int numOfRobots;
        int robotsFired = 0;

        RobotNode* head;
        RobotNode* destroyedHead;
        CustomQueue respawnQueue;

    public:
        Battlefield() : head(nullptr), destroyedHead(nullptr) {}
        ~Battlefield() {
            while (head) {
                RobotNode* temp = head;
                head = head->next;
                delete temp->robot;
                delete temp;
            }
            while (destroyedHead) {
                RobotNode* temp = destroyedHead;
                destroyedHead = destroyedHead->next;
                delete temp->robot;
                delete temp;
            }
        }

        int get_NUM_OF_COLS() const { return NUM_OF_COLS; }
        int get_NUM_OF_ROWS() const { return NUM_OF_ROWS; }
        int get_turns() const { return turns; }
        int get_numOfRobots() const { return numOfRobots; }

        RobotNode* getHead() const { return head; }
        RobotNode* getDestroyedHead() const { return destroyedHead; }

        void addRobot(Robot* robot) {
            RobotNode* newNode = new RobotNode(robot);
            newNode->next = head;
            head = newNode;
        }

        void removeRobot(RobotNode* prevNode, RobotNode* node) {
            if (prevNode == nullptr) {
                head = node->next;
            } else {
                prevNode->next = node->next;
            }
            node->next = destroyedHead;
            destroyedHead = node;
        }

        void eraseRobot(RobotNode* prevNode, RobotNode* node) {
            if (node->robot->robotLives() <= 0) {
                if (prevNode == nullptr) {
                    head = node->next;
                } else {
                    prevNode->next = node->next;
                }
                delete node->robot;
                delete node;
            } else {
                if (prevNode == nullptr) {
                    head = node->next;
                } else {
                    prevNode->next = node->next;
                }
                node->next = nullptr;
                respawnQueue.enqueue(node->robot);
                delete node;
            }
        }

        void readFile(string filename) {
            cout << "Reading file: " << filename << endl;
            string skip;
            string robotType = "";
            string robotName = "";
            string strR = "";
            string strC = "";
            int r = 0;
            int c = 0;

            ifstream inputFile;
            inputFile.open(filename);
            if (!inputFile) {
                cout << "Can't open " << filename << " file" << endl;
                return;
            }

            inputFile >> skip >> skip >> skip >> NUM_OF_COLS >> NUM_OF_ROWS;
            cout << "M by N: " << NUM_OF_COLS << " " << NUM_OF_ROWS << endl;
            inputFile >> skip >> turns;
            cout << "Turns: " << turns << endl;
            inputFile >> skip >> numOfRobots;
            cout << "Robots: " << numOfRobots << endl << endl;


            int robotRows[numOfRobots];
            int robotCols[numOfRobots];
            int currentIndex = 0;

            for (int i = 0; i < numOfRobots; ++i) {
                inputFile >> robotType >> robotName >> strC >> strR; // Switch order to (x, y)

                bool positionOccupied;
                do {
                    if (strR == "random") {
                        r = rand() % NUM_OF_ROWS;
                    } else {
                        r = stoi(strR);
                    }

                    if (strC == "random") {
                        c = rand() % NUM_OF_COLS;
                    } else {
                        c = stoi(strC);
                    }

                    positionOccupied = false;
                    for (int j = 0; j < currentIndex; ++j) {
                        if (robotRows[j] == r && robotCols[j] == c) {
                            positionOccupied = true;
                            break;
                        }
                    }
                } while (positionOccupied);


                robotRows[currentIndex] = r;
                robotCols[currentIndex] = c;
                currentIndex++;


                RobotNode* newNode;
                if (robotType == "GenericRobot") {
                    newNode = new RobotNode(new GenericRobot(r, c, robotType, robotName));
                } else if (robotType == "BomberBot") {
                    newNode = new RobotNode(new BomberBot(r, c, robotType, robotName));
                } else if (robotType == "AngelBot") {
                    newNode = new RobotNode(new AngelBot(r, c, robotType, robotName));
                }

                if (head == nullptr) {
                    head = newNode;
                } else {
                    RobotNode* current = head;
                    while (current->next != nullptr) {
                        current = current->next;
                    }
                    current->next = newNode;
                }
                cout << robotType << " " << robotName << " deployed at location (" << c << "," << r << ")" << endl; // Adjust output
            }
            inputFile.close();


            placeRobot();
        }

        void placeRobot() {
            string battlefieldGrid[NUM_OF_ROWS][NUM_OF_COLS];
        
            // Initialize the grid with empty cells
            for (int i = 0; i < NUM_OF_ROWS; ++i) {
                for (int j = 0; j < NUM_OF_COLS; ++j) {
                    battlefieldGrid[i][j] = ".";
                }
            }
        
            // Traverse the robot list and place robots on the grid
            RobotNode* current = head;
            while (current) {
                Robot* robot = current->robot;
        
                // Check if the robot is hiding
                HideBot* hideBot = dynamic_cast<HideBot*>(robot);
                if (hideBot && hideBot->isHidden()) {
                    // Skip placing the robot on the grid if it is hiding
                    current = current->next;
                    continue;
                }
        
                int r = robot->r();
                int c = robot->c();
                string type = robot->robotType();
        
                if (r >= 0 && r < NUM_OF_ROWS && c >= 0 && c < NUM_OF_COLS) {
                    if (type == "GenericRobot") {
                        battlefieldGrid[r][c] = "G";
                    } else if (type == "BomberBot") {
                        battlefieldGrid[r][c] = "B";
                    } else if (type == "AngelBot") {
                        battlefieldGrid[r][c] = "A";
                    }
                } else {
                    cerr << "Robot " << robot->robotName() << " has invalid coordinates." << endl;
                }
        
                current = current->next;
            }
        
            // Print the grid with the y-axis (rows) starting from the top
            for (int i = NUM_OF_ROWS - 1; i >= 0; --i) {
                // Print row number (y-axis)
                cout << setw(2) << i << " ";
                for (int j = 0; j < NUM_OF_COLS; ++j) {
                    cout << setw(2) << battlefieldGrid[i][j] << " ";
                }
                cout << endl;
            }
        
            // Print column numbers (x-axis) at the bottom
            cout << "   ";
            for (int j = 0; j < NUM_OF_COLS; ++j) {
                cout << setw(2) << j << " ";
            }
            cout << endl;
        }

        void runSimulation() {
            ofstream logFile("robot_status.txt"); // Open file for writing

            if (!logFile) {
                cerr << "Unable to open file for writing" << endl;
                return;
            }

            for (int turn = 0; turn < turns; ++turn) {
                cout << endl << "------------------------Turn " << turn + 1 << "------------------------" << endl;
                logFile << "-Turn " << turn + 1 << "-" << endl; // Log turn number

                // Respawn only one robot from respawnQueue per turn
                if (!respawnQueue.isEmpty()) {
                    Robot* robot = respawnQueue.dequeue();
                    int r = rand() % NUM_OF_ROWS;
                    int c = rand() % NUM_OF_COLS;
                    robot->robotRespawn(r, c);
                    robot->setAmmo(10); // Reset ammo on respawn
                    addRobot(robot);
                    cout << robot->robotType() << " " << robot->robotName() << " respawned at location (" << c << "," << r << ")" << endl;
                }

                // Check if there is a single robot left and the respawn queue is empty
                if (head != nullptr && head->next == nullptr && respawnQueue.isEmpty()) {
                    cout << "Single robot left on the battlefield: " << head->robot->robotType() << " " << head->robot->robotName() << ". This robot wins! Simulation ends." << endl;
                    logFile << "Single robot left on the battlefield: " << head->robot->robotType() << " " << head->robot->robotName() << ". This robot wins! Simulation ends." << endl;
                    break;
                }

                // Check if no robots are left on the battlefield
                if (head == nullptr) {
                    cout << "No more robots left. Simulation ends." << endl;
                    logFile << "No more robots left. Simulation ends." << endl;
                    break;
                }

                RobotNode* prev = nullptr;
                RobotNode* current = head;
                while (current) {
                    Robot* robot = current->robot;
                    cout << "TURN " << robot->robotType() << " " << robot->robotName() << " TO MOVE" << endl;
                    logFile << *robot << endl; // Log robot status including ammo

                    robot->action(*this);

                    if (robot->robotLives() <= 0 || robot->getAmmo() == 0) {
                        if (robot->getAmmo() == 0) {
                            robot->lostLive(); // Deduct life if out of ammo
                        }
                        cout << robot->robotType() << " " << robot->robotName() << " is destroyed and will respawn next round." << endl;
                        RobotNode* toDestroy = current;
                        current = current->next;
                        eraseRobot(prev, toDestroy); // Move robot to respawn queue
                    } else {
                        prev = current;
                        current = current->next;
                    }

                    placeRobot();
                    cout << "PRESS ENTER TO CONTINUE";
                    cin.ignore();

                    logFile << *robot << endl; // Log robot's current status
                    cout << endl;
                    logFile << endl;
                }

                logFile << endl << endl;
            }

            logFile.close(); // Close the file
        }
};

void ThinkingRobot::upgradeTree() {
    if (v.empty()) {
        v = {0, 0, 0}; // Initialize the state to 0,0,0
    }

    // Collect indices of unoccupied slots (slots with value 0)
    vector<int> availableSlots;
    for (int i = 0; i < 3; ++i) {
        if (v[i] == 0) {
            availableSlots.push_back(i);
        }
    }

    // If no slots are available, no further upgrades can be applied
    if (availableSlots.empty()) {
        cout << "All upgrade slots are occupied. No further upgrades available." << endl;
        return;
    }

    // Randomly select one of the available slots
    int slot = availableSlots[rand() % availableSlots.size()];

    // Apply the upgrade to the selected slot
    switch (slot) {
    case 0: // First slot
        v[0] = (rand() % 2) + 1; // Randomly choose 1 or 2
        break;
    case 1: // Second slot
        v[1] = (rand() % 3) + 1; // Randomly choose 1, 2, or 3
        break;
    case 2: // Third slot
        v[2] = (rand() % 2) + 1; // Randomly choose 1 or 2
        break;
    }

    // Display the updated state
    cout << "Upgrade state: (" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;

    // Apply specific upgrades based on the state
    if (v[0] == 1) {
        cout << "HideBot upgrade unlocked!" << endl;
        setUpgradeType("hidebot");
    } else if (v[0] == 2) {
        cout << "JumpBot upgrade unlocked!" << endl;
        setUpgradeType("jumpbot");
    }

    if (v[1] == 1) {
        cout << "LongShotBot upgrade unlocked!" << endl;
        setUpgradeType("longshotbot");
    } else if (v[1] == 2) {
        cout << "SemiAutoBot upgrade unlocked!" << endl;
        setUpgradeType("semiautobot");
    } else if (v[1] == 3) {
        cout << "ThirtyShotBot upgrade unlocked!" << endl;
        setUpgradeType("thirtyshotbot");
    }

    if (v[2] == 1) {
        cout << "ScoutBot upgrade unlocked!" << endl;
        setUpgradeType("scoutbot");
    } else if (v[2] == 2) {
        cout << "TrackBot upgrade unlocked!" << endl;
        setUpgradeType("trackbot");
    }
}

void BomberBot::throwBomb(Battlefield& bf) {
    // ...existing code...
    if (ammo == 0) {
        cout << type << " " << name << " has no bombs left to throw!" << endl;
        return;
    }
    // ...existing code...
    int bombC = rand() % bf.get_NUM_OF_COLS();
    int bombR = rand() % bf.get_NUM_OF_ROWS(); // Switch to (x, y)

    cout << type << " " << name << " throws a bomb at (" << bombC << "," << bombR << ")" << endl;

    const int blastRadius = 1;
    bool hitReported = false; // To ensure only one hit is shown in the output

    for (int i = bombR - blastRadius; i <= bombR + blastRadius; ++i) {
        for (int j = bombC - blastRadius; j <= bombC + blastRadius; ++j) {
            if (i >= 0 && i < bf.get_NUM_OF_ROWS() && j >= 0 && j < bf.get_NUM_OF_COLS() && !(i == positionR && j == positionC)) {
                RobotNode* prev = nullptr;
                RobotNode* current = bf.getHead();

                while (current) {
                    if (current->robot->r() == i && current->robot->c() == j && current->robot != this) {
                        int accuracy = rand() % 100 + 1; // Generate random number between 1 and 100
                        if (accuracy <= 2) { // Hit
                            current->robot->lostLive();
                            kills++;
                            if (!hitReported) {
                                cout << type << " " << name << " bombed (" << j << "," << i << ") and hit "
                                     << current->robot->robotName() << "!" << endl;
                                cout << type << " now has " << kills << " kills." << endl;
                                hitReported = true;
                            }
                            bf.eraseRobot(prev, current);
                        } else { // Miss
                            if (!hitReported) {
                                cout << type << " " << name << " bombed (" << j << "," << i << ") but missed!" << endl;
                                cout << "There was a robot (" << current->robot->robotName() << ") at the location." << endl;
                                hitReported = true;
                            }
                        }
                        break;
                    } else {
                        prev = current;
                        current = current->next;
                    }
                }
            }
        }
    }

    if (!hitReported) {
        cout << type << " " << name << " bombed (" << bombC << "," << bombR << ") but there was no robot there." << endl;
    }
}

void MovingRobot::move(Battlefield& bf) {
    const vector<pair<int, int>> moves = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    bool validMove = false;
    while (!validMove) {
        int moveIndex = rand() % moves.size();
        int newR = positionR + moves[moveIndex].first;
        int newC = positionC + moves[moveIndex].second;

        bool outOfBound = (newR < 0 || newR >= bf.get_NUM_OF_ROWS() || newC < 0 || newC >= bf.get_NUM_OF_COLS());

        if (!outOfBound) {
            bool collision = false;
            RobotNode* current = bf.getHead();
            while (current) {
                if (current->robot->r() == newR && current->robot->c() == newC) {
                    collision = true;
                    break;
                }
                current = current->next;
            }
        if (!collision) {
                positionR = newR;
                positionC = newC;
                validMove = true;
            }
        }
    }
    cout << type << " " << name << " moves to (" << positionC << "," << positionR << ")" << endl; // Adjust output
}

void SeeingRobot::look(Battlefield& bf) {
    int nearbyRobots = 0;

    const vector<pair<int, int>> lookDirections = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    for (const auto& dir : lookDirections) {
        int newR = positionR + dir.first;
        int newC = positionC + dir.second;

        if (newR >= 0 && newR < bf.get_NUM_OF_ROWS() && newC >= 0 && newC < bf.get_NUM_OF_COLS()) {
            RobotNode* current = bf.getHead();
            while (current) {
                if (current->robot->r() == newR && current->robot->c() == newC) {
                    nearbyRobots++;
                }
                current = current->next;
            }
        }
    }

    if (nearbyRobots == 0) {
        cout << type << " " << name << " look around but no robot around" << endl;
    } else {
        cout << type << " " << name << " look around and there have robots nearby" << endl;
    }
}


void ShootingRobot::fire(Battlefield& bf, const string& fireMethod) {
    if (!consumeAmmo()) {
        return; // Stop firing if no ammo left
    }

    if (fireMethod == "genericrobot") {
        const vector<pair<int, int>> fireDirections = {
            {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
        };

        bool targetFound = false;

        for (const auto& dir : fireDirections) {
            int targetR = positionR + dir.first;
            int targetC = positionC + dir.second;

            if (targetR >= 0 && targetR < bf.get_NUM_OF_ROWS() && targetC >= 0 && targetC < bf.get_NUM_OF_COLS()) {
                RobotNode* prev = nullptr;
                RobotNode* current = bf.getHead();

                while (current) {
                    if (current->robot->r() == targetR && current->robot->c() == targetC) {
                        targetFound = true;
                        int accuracy = rand() % 100 + 1; // Generate random number between 1 and 100
                        if (accuracy <= 70) { // Hit
                            current->robot->lostLive();
                            kills++;
                            cout << type << " " << name << " fired at (" << targetC << "," << targetR << ") and hit "
                                 << current->robot->robotName() << "!" << endl;
                            cout << type << " " << name << " now has " << kills << " kills." << endl;
                            upgradeTree();
                            bf.eraseRobot(prev, current);
                        } else { // Miss
                            cout << type << " " << name << " fired at (" << targetC << "," << targetR << ") but missed!" << endl;
                            cout << "There was a robot (" << current->robot->robotName() << ") at the location." << endl;
                        }
                        return;
                    } else {
                        prev = current;
                        current = current->next;
                    }
                }
            }
        }

        if (!targetFound) {
            cout << type << " " << name << " fired but there was no robot within range." << endl;
        }
    } else if (fireMethod == "longshotbot") {
        const vector<pair<int, int>> fireDirections = {
            {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
        };

        bool targetFound = false;

        for (const auto& dir : fireDirections) {
            for (int step = 1; step <= 3; ++step) {
                int targetR = positionR + dir.first * step;
                int targetC = positionC + dir.second * step;

                if (targetR >= 0 && targetR < bf.get_NUM_OF_ROWS() && targetC >= 0 && targetC < bf.get_NUM_OF_COLS()) {
                    RobotNode* prev = nullptr;
                    RobotNode* current = bf.getHead();

                    while (current) {
                        if (current->robot->r() == targetR && current->robot->c() == targetC) {
                            targetFound = true;
                            int accuracy = rand() % 100 + 1; // Generate random number between 1 and 100
                            if (accuracy <= 50) { // Lower accuracy due to longer range
                                current->robot->lostLive();
                                kills++;
                                upgradeTree();
                                cout << type << " " << name << " fired at (" << targetC << "," << targetR << ") and hit "
                                     << current->robot->robotName() << "!" << endl;
                                cout << type << " " << name << " now has " << kills << " kills." << endl;
                                bf.eraseRobot(prev, current);
                            } else { // Miss
                                cout << type << " " << name << " fired at (" << targetC << "," << targetR << ") but missed!" << endl;
                                cout << "There was a robot (" << current->robot->robotName() << ") at the location." << endl;
                            }
                            return;
                        } else {
                            prev = current;
                            current = current->next;
                        }
                    }
                }
            }
        }
        } else if (fireMethod == "semiautobot") {

            const vector<pair<int, int>> fireDirections = {
            {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
        };

        bool targetFound = false;
            for (const auto& dir : fireDirections) {
                    int targetR = positionR + dir.first;
                    int targetC = positionC + dir.second;

                    if (targetR >= 0 && targetR < bf.get_NUM_OF_ROWS() && targetC >= 0 && targetC < bf.get_NUM_OF_COLS()) {
                        RobotNode* prev = nullptr;
                        RobotNode* current = bf.getHead();

                    while (current) {
                        if (current->robot->r() == targetR && current->robot->c() == targetC) {
                            targetFound = true;

                            int hits = 0;
                            for (int i = 0; i < 3; ++i) {
                                int shotAccuracy = rand() % 100 + 1;
                                if (shotAccuracy <= 70) hits++;
                        }

                        if (hits > 0) {
                            current->robot->lostLive();
                            kills++;
                            cout << type << " " << name << " semi-auto fired 3 shots at (" << targetC << "," << targetR << ") and hit with " << hits << " shot(s)!" << endl;
                            cout << type << " " << name << " now has " << kills << " kills." << endl;
                            upgradeTree();
                            bf.eraseRobot(prev, current);
                        } else {
                            cout << type << " " << name << " semi-auto fired 3 shots at (" << targetC << "," << targetR << ") but all missed!" << endl;
                            cout << "There was a robot (" << current->robot->robotName() << ") at the location." << endl;
                        }

                        return; // Stop after firing at the first valid target
                    }

                    prev = current;
                    current = current->next;
                }
            }
        }



        if (!targetFound) {
            cout << type << " " << name << " fired but there was no robot within 3-unit range." << endl;
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    Battlefield battlefield;
    battlefield.readFile("robot.txt");
    battlefield.runSimulation();

    return 0;
}
