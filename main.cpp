#define ANALYSER    0
#define GETTOSEARCH 1
#define INITSEARCH  2
#define DRILL       3
#define RISE        4
#define RETURN      5
#define EVAC        6
#define EVAC2       7
#define LEFT        9
#define RIGHT       10
#define UP          11
#define DOWN        12

float attothervector[3];
float targetRate[3];
int locationC;
int direction;
float distInt;
float position[3];
bool isTop;

float concentrations[5];
int invIndex;

void init()
{

    float myZRState[12];
    api.getMyZRState(myZRState);
    

    position[0] = myZRState[0];
    position[1] = myZRState[0];
    position[2] = 0.36f;

    direction = LEFT;

    distInt = 0.16f;
    locationC = ANALYSER;
    isTop = true;
}



int findClosestItem() {
    float myZRState[12];
    api.getMyZRState(myZRState);
    int closestItem = -1;
    float closestDist = 10000.0f;
    //Get Satellite Location
    float satLoc[] = {myZRState[0],myZRState[1],myZRState[2]};
    
    for(int i = 0; i < 9; i++) {
    }
    return closestItem;
}
float getDistance (float pos[3]){
    float myZRState[12];
    ZeroRoboticsGame game = ZeroRoboticsGame::instance();
    api.getMyZRState(myZRState);
    //Get Satellite Location
    float satLoc[] = {myZRState[0],myZRState[1],myZRState[2]};
    
    //Get resultant vector
    float rVector[3];
    mathVecSubtract(rVector, pos, satLoc, 3);
    
    //Get distance between satellite and item
    float distance = mathVecMagnitude(rVector, 3);
    
    return distance;
}
bool withinBoundary(float pos[3]) {
    if(pos[0] < -0.48 || pos[0] > 0.48) {
        return false;
    }
    if(pos[1] < -0.64 || pos[1] > 0.64) {
        return false;
    }
    return true; 
}
bool moveTo(float pos[3])
{
    float myZRState[12];
    api.getMyZRState(myZRState);
    
    //Get Satellite Location
    float satLoc[] = {myZRState[0],myZRState[1],myZRState[2]};
    
    //Get Satellite Velocity
    float satVelocity[] = {myZRState[3],myZRState[4],myZRState[5]};
    

    //Get resultant vector
    float rVector[3];
    mathVecSubtract(rVector, pos, satLoc, 3);
    
    //Get distance between satellite and item
    float distance = mathVecMagnitude(rVector, 3); 

    //
    if(distance <  mathVecMagnitude(satVelocity, 3) * 6.0f) {
        api.setPositionTarget(pos);
    }
    else {
        api.setVelocityTarget(rVector);
    }
    
    if(distance < 0.01f && mathVecMagnitude(satVelocity, 3) < 0.01f ) {
        return true;
    }
    return false;
}

bool drill() {
    float myZRState[12];
    ZeroRoboticsGame game = ZeroRoboticsGame::instance();
    api.getMyZRState(myZRState);
    
    float velocityT[3] = {0.0f, 0.0f, 0.0f};
    api.setVelocityTarget(velocityT);
    float attitudeVector[3];
    attitudeVector[0] = 0;
    attitudeVector[1] = 0;
    attitudeVector[2] = 1.8;
    
    if(game.getDrillError()) {
        game.stopDrill();
        locationC = GETTOSEARCH;
        return false;
    }
    
    if(!game.getDrillEnabled()) {    
        if(game.startDrill()){
            api.setAttRateTarget(attitudeVector);
        }
        else{
            DEBUG(("Drill not starting!!!!"));
        }
    }
    else {
        if(game.checkSample()){
            invIndex = game.pickupSample();
            return true;
        }
    }
    return false;
}
void search() {
    DEBUG(("INITSEARCH"));
    if(direction == LEFT) {
        DEBUG(("LEFT"));
        position[0] = position[0] - distInt;
        if(!withinBoundary(position)) {
            if(isTop) {
                direction = DOWN;
            }
            else {
                direction = UP;
            }
            position[0] = position[0] + distInt;
        }
    }
    if(direction == RIGHT) {
        DEBUG(("RIGHT"));
        position[0] = position[0] + distInt;
        if(!withinBoundary(position)) {
            if(isTop) {
                direction = DOWN;
            }
            else {
                direction = UP;
            }
            position[0] = position[0] - distInt;
        }
    }
    if(direction == UP) {
        DEBUG(("UP"));
        position[1] = position[1] + distInt;
        if(!withinBoundary(position)) {
            position[1] = position[1] - distInt;
            direction = DOWN;
        }
        else {
            if(getDistance(position) < 0.1f) {
                float tmp[] = {position[0] + distInt, position[1], position[2]};
                if(withinBoundary(tmp)) {
                    direction = RIGHT;
                }
                tmp[0] = position[0] - distInt;
                tmp[1] = position[1]; 
                tmp[2] = position[2];
                if(withinBoundary(tmp)) {
                    direction = LEFT;
                }
            }
        }
    }
    if(direction == DOWN) {
        DEBUG(("DOWN"));
        position[1] = position[1] - distInt;
        if(!withinBoundary(position)) {
            position[1] = position[1] + distInt;
            direction = UP;
        }
        else {
            if(getDistance(position) < 0.1f) {
                float tmp[] = {position[0] + distInt, position[1], position[2]};
                if(withinBoundary(tmp)) {
                    direction = RIGHT;
                }
                tmp[0] = position[0] - distInt;
                tmp[1] = position[1]; 
                tmp[2] = position[2];
                if(withinBoundary(tmp)) {
                    direction = LEFT;
                }
            }
        }
    }
    
    else if(game.getNumSamplesHeld() == 3) {
        game.stopDrill();
        float origin[] = {0,0,0};
        moveTo(origin);
        if(getDistance(origin) < 0.05f) {
            for(int i = 0; i < 3; i++) {
                game.dropSample(i);
            }
        }
    }
}
bool rise() {
    float myZRState[12];
    ZeroRoboticsGame game = ZeroRoboticsGame::instance();
    api.getMyZRState(myZRState);
    
    position[2] = 0.30f;
    float attitude[] = {0.0f,0.0f,0.0f};
    api.setAttRateTarget(attitude);
    if(moveTo(position)) {
        return true;
    }
    return false;
}
void loop(){
    float myZRState[12];
    ZeroRoboticsGame game = ZeroRoboticsGame::instance();
    api.getMyZRState(myZRState);
    if (locationC == EVAC) {
        search();
        position[2] = 0.40f;
        locationC = EVAC2;
        DEBUG(("EVAC2"));
    }
    if (locationC == EVAC2) {
        float attitude[] = {0.0f,0.0f,0.0f};
        api.setAttRateTarget(attitude);
        if(moveTo(position)){
            locationC = RISE;
            DEBUG(("RISE"));
        }
    }
    if (locationC == ANALYSER) {
        DEBUG(("ANALYSER"));
        locationC = GETTOSEARCH;
    }
    if (locationC == RISE) {
        if(rise()) {
            locationC = GETTOSEARCH;
            position[2] = 0.36f;
        }
    }
    if (locationC == RETURN) {
        float basePos[] = {0,0,0};
        float attitude[] = {0.0f,0.0f,-1.0f};
        api.setAttitudeTarget(attitude);
        if(moveTo(basePos) && game.atBaseStation()) {
            for(int i = 0; i < 5; i++) {
                game.dropSample(i);
                DEBUG(("GETTOSEARCH"));
                locationC = GETTOSEARCH;
            }

        }
    }
    if (locationC == GETTOSEARCH) {
        DEBUG(("GETTOSEARCH"));
        search();
        
        position[2] = 0.36f;
        locationC = INITSEARCH;
    }
    if(locationC == INITSEARCH) {
        DEBUG(("INITSEARCH"));
        if(moveTo(position)){
            locationC = DRILL;
            DEBUG(("DRILL"));
        }
    }
    if(locationC == DRILL) {
        bool gotSample = drill();
        float tmpPos[] = {position[0],position[1]};
        if(game.isGeyserHere(tmpPos)) {
                game.stopDrill();
                locationC = EVAC;
                DEBUG(("EVAC"));
            }
        else if(gotSample == true) {

            if(game.getNumSamplesHeld() >= 5) {
                game.stopDrill();
                locationC = RETURN;
            }
            
            else if (game.getDrills(tmpPos) >= 3) {
                game.stopDrill();
                locationC = RISE;
            }
        }
    }
}
