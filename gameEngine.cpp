#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>

#include "gameEngine.h"

#define MAX_ROUNDS 5

string initTileBag;
string function;
string param1, param2, param3;
int n = 0;

bool isLoading = false;

GameEngine::GameEngine() {
    //Create and fill TileBag
    /*Only fill the TileBag if it is a new game, otherwise go on normally,
    and the tilebag will be filled on runGame() */
    tileBag = new TileBag();

    if(isLoading == false) {
        initTileBag = tileBag->generateTileBag("RYLYRLRLLLULYYLULYUURYBYYLRUYBLUYULBRUUUUBURRBRRYBYBBUBYRRRLBRULBRYUYRBUULBYYLLBLRLYRUUBRBUYBYLBBLBR");
        tileBag->generateTileBag("RYLYRLRLLLULYYLULYUURYBYYLRUYBLUYULBRUUUUBURRBRRYBYBBUBYRRRLBRULBRYUYRBUULBYYLLBLRLYRUUBRBUYBYLBBLBR");
    }

    //Create and fill factories
    factories = new Factories();
    factories->FillFactoriesFromTileBag(tileBag);
}

GameEngine::GameEngine(const GameEngine& other) {
    tileBag = new TileBag(*other.tileBag);
    factories = new Factories(*other.factories);

    turns = other.turns;

    for(int i = 0; i < TOTAL_PLAYERS; ++i) {
        players[i] = new Player(*other.players[i]);
    }

}

GameEngine::~GameEngine() {
    delete tileBag;
    delete factories;

    for(int i = 0; i < TOTAL_PLAYERS; ++i) {
        if(players[i] != nullptr) {
            delete players[i];
            players[i] = nullptr;
        }
    }
}

void GameEngine::runGame(Load* load) {
    this->load = load;

    initTileBag = tileBag->generateTileBag(load->getTileBag());
    tileBag->generateTileBag(load->getTileBag());

    runGame();
}

void GameEngine::runGame() {
    string player1Name;
    string player2Name;

    if(isLoading == true) {
        //TESTING
        // cout << "TileBag: " + load->getTileBag() << endl;
        // cout << "Player1: " + load->getPlayer1() << endl;
        // cout << "Player2: " + load->getPlayer2() << endl;

        //Create players
        player1Name = load->getPlayer1();
        player2Name = load->getPlayer2();
        createPlayers(player1Name, player2Name);

    } else {
        cout << "Enter a name for Player 1" << endl;
        cout << ">";
        cin >> player1Name;

        if(cin.good()) {
            cout << "Enter a name for Player 2" << endl;
            cout << ">";
            cin >> player2Name;

            if(cin.good()) {
                createPlayers(player1Name, player2Name);
            }
        } else {

            cout << "Input was not accepted, please try again." << endl;
        }
        cout << endl;

        cout << "Let's Play!" << endl;
        cout << endl;
    }    
    //Run Round
    bool keepPlaying = true;
    bool firstPlayerTurn = true;
    Player* currentPlayer;

    //Run Rounds
    int rounds = 0;
    while(keepPlaying && !cin.eof() && rounds < MAX_ROUNDS) {
        if(isLoading == false) {
            cout << "=== Start Round " << rounds + 1 <<" ===" << endl;
        }

        //Run Single Round:
        while(keepPlaying && !cin.eof() && !(factories->allFactoriesAreEmpty())) {
        //Check which players turn it is:
            if(firstPlayerTurn) {
                currentPlayer = players[0];
            } else {
                currentPlayer = players[1];
            }
            //Flip the player turn for next turn.
            firstPlayerTurn = !firstPlayerTurn;
            keepPlaying = runTurn(currentPlayer);
        }

        //Fill the factories back up
        factories->FillFactoriesFromTileBag(tileBag);

        //Move tiles from mosaic to patternline for all players
        for(int i = 0; i < TOTAL_PLAYERS; ++i) {
            addTilesToMosaicFromPatternLine(players[i]);
        }

        //Update Scoring
        for(int i = 0; i < TOTAL_PLAYERS; ++i) {
            players[i]->setPlayerScore(calculatePlayerScores(players[i]));
        }
            
        //Increment round
        ++rounds;
    }
}

bool GameEngine::runTurn(Player* currentPlayer) {
    bool keepPlaying = true;

    if(isLoading == false) {
        cout << "TURN FOR PLAYER: " << currentPlayer->getPlayerName() << endl;

        //Display Factories:
        cout << "Factories: " << endl;
        printFactories();

        cout << "Mosaic for " << currentPlayer->getPlayerName() << ": " << endl;
        printPlayerMosaic(currentPlayer);
        cout << endl;

        cout << "> ";
        cin >> function;

        //Get player input 
        if(playerEntersTurn(currentPlayer, function)) {
            turns.back();
        } else {
            keepPlaying = false;
        }
    } else {

        //TESTING
        std::istringstream iss(load->getTurns()[n]); 
        vector<string> splitTurn(std::istream_iterator<string>{iss}, std::istream_iterator<string>());
        function = splitTurn[0];
        param1 = splitTurn[1];
        if(function == "turn") {
            param2 = splitTurn[2];
            param3 = splitTurn[3];
        }
        //END TESTING

        //Pass through the turn from save file
        //TODO: get a string representing a turn "turn 3 L 3" and split it into function + params 1-4.
        if(playerEntersTurn(currentPlayer, function, param1, param2, param3)) {
            turns.back();
            n++;
            if(n >= load->getTurns().size()) {
                isLoading = false;
            }
        } else {
            keepPlaying = false;
        }
    }
    
    return keepPlaying;
}

void GameEngine::printFactories() {
    for(int i = 0; i < NUMBER_OF_FACTORIES; ++i) {
        Factory* current = factories->getFactory(i);

        std::cout << i << ": ";
        for(int j = 0; j < current->size(); ++j) {
            std::cout << current->getTileAt(j)->getCharColour() << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void GameEngine::createPlayers(string player1Name, string player2Name) {
    players[0] = new Player(player1Name);
    players[1] = new Player (player2Name);
}

bool GameEngine::playerEntersTurn(Player* currentPlayer, string function,  string param1, string param2, string param3) {
    bool turnEntered = false;
    
    bool invalidInput = true;
    do {
        invalidInput = true;

        if(cin.eof()) {
            invalidInput = false;
            turnEntered = false;
        
        } else {
            // if the user types "save x x x" only take the first x value as the fileName and pass
            if(function == "save") {
                if(isLoading == true) {
                    saveGame(param1);
                    invalidInput = false;
                } else {
                    string fileName;
                    cin >> fileName;
                    saveGame(fileName);
                    invalidInput = false;
                }

            // if the first word the player types is (turn)
            } else if(function == "turn") {
                char colour;
                int factoryNumber, patternLineRow;

                // if a game is being loaded, play a simplified version with no cout
                if(isLoading == true) {
                    factoryNumber = std::stoi(param1);
                    colour = param2[0];
                    patternLineRow = std::stoi(param3);
                // if the player types 'turn x y z', use x,y,z and take the turn.
                } else {
                    cin >> factoryNumber;
                    cin >> colour;
                    cin >> patternLineRow;
                }


                if(cin.good()) {
                    //Process turns
                    if(!addTileFromFactoryToMosaic(currentPlayer, factoryNumber, colour, patternLineRow)) {
                        invalidInput = true;
                    } else {
                        //Save turn variable for saving
                        turns.push_back(function + " " + std::to_string(factoryNumber) + " " + colour + " " + std::to_string(patternLineRow));

                        turnEntered = true;
                        invalidInput = false;

                        // TESTING METHOD: Displays player's updated mosaic
                        // cout << currentPlayer->getPlayerName() << "'s Updated Mosaic: " << endl;
                        // printPlayerMosaic(currentPlayer);
                    }

                } else {
                    if(isLoading == false) {
                        cout << "Invalid turn entered. Should enter <Factory Row Number> <Colour> <Mosaic Row>" << endl;
                        cout << "EXAMPLE: > turn 2 B 3" << endl;
                    } 
                    invalidInput = true;

                    //Clear current input, so user can re enter input.
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }

            } else {
                if(isLoading == false) {
                    cout << "error: unknown function defined, please try again." << endl;
                }
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            
            cout << endl;
        }
        
    } while (invalidInput);

    return turnEntered;
}

bool GameEngine::addTileFromFactoryToMosaic(Player* currentPlayer, int factoryNumber, char colour, int pattRow) {
    bool validInput = true;
    Colour tileColour =  convertCharToColour(toupper(colour));
    Mosaic* mosaic = currentPlayer->getMosaic();
    int patternLineRow = pattRow - 1;

    //Validate input
    if(validateTurnInput(currentPlayer, factoryNumber, colour, patternLineRow)) {
        if(isLoading == false) {
            std::cout << "Turn successful." << endl;
        }
        Factory* factory = factories->getFactory(factoryNumber);
        int factoryColourIndex = factory->getIndexOfSameColourTile(tileColour);

        //If factory still contains Tile colour keep adding to mosaic
        while(factoryColourIndex != INVALID_INDEX) {
            //If patternline is not full add there, otherwise add rest to broken tiles
            Tile* addTile = factory->getTileAt(factoryColourIndex);

            if(!mosaic->patternLineFull(patternLineRow)) {
                //Add tile to pattern line
                mosaic->addTileToPatternLine(addTile, patternLineRow);
            } else {
                mosaic->addBrokenTiles(addTile);
            }

            //Remove tile from factory
            factory->removeTileAt(factoryColourIndex);

            //Update factory index
            factoryColourIndex = factory->getIndexOfSameColourTile(tileColour);
        }

        //Empty the rest of the tiles within the chosen factory
        if(factoryNumber != 0) {
            //Add rest of tiles to centre factory
            while(factory->size() > 0) {
                //Keep adding front tile into centre factory
                factories->getFactory(0)->addTile(factory->getTileAt(0));
                factory->removeTileAt(0);
            }

        } else {
            //Add rest of tiles to end of tileBag
            while(factory->size() > 0) {
                tileBag->addTile(factory->getTileAt(0));
                factory->removeTileAt(0);
            }

        } 

    } else {
        validInput = false;
    }

    return validInput;
}

bool GameEngine::validateTurnInput(Player* currentPlayer, int factoryNumber, char colour, int patternLineRow) {
    bool validTurn = true;

    //Ensure facotryNumber and colour is valid
    Colour tileColour =  convertCharToColour(toupper(colour));
    Mosaic* mosaic = currentPlayer->getMosaic();

    //Check if factory and colour within range - then check if colour exists in factory
    if(factoryNumber < 0 || factoryNumber >= NUMBER_OF_FACTORIES) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Invalid factoryNumber was given. Should be between 0 and " << FACTORY_SIZE << endl;
        }
    } else if(tileColour == BLANK || tileColour == NO_TILE) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Invalid colour was entered. Enter one of the following: R Y B L U F" << endl;
        }
    } else if(factories->getFactory(factoryNumber)->getIndexOfSameColourTile(tileColour) == INVALID_INDEX) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Given colour does not exist in chosen factory." << endl;   
        }
    }
    
    //Check that factory contains tiles
    if (factories->getFactory(factoryNumber)->size() == 0) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Selected factory is empty" << endl;
        }
    }
    
    //Check that patternline is within range, is not already full, and is of matching colour.
    if(patternLineRow < 0 || patternLineRow >= ROWS) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Invalid patternLine row was given. Should be between 0 and " << ROWS << endl;
        }
    } else if(mosaic->patternLineFull(patternLineRow)) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Chosen patternLine row is full, please choose a different row." << ROWS << endl;
        }
    } else if(mosaic->getPatternLineColour(patternLineRow) != NO_TILE && mosaic->getPatternLineColour(patternLineRow) != tileColour) {
        validTurn = false;
        if(isLoading == false) {
            cout << "Invalid colour. Chosen pattern line already contains tiles of a different colour." << endl;
        }
    }

    //Check if grid already contains colour in a given patternline.
    for(int colm = 0; colm < COLS; ++colm) {
        if(mosaic->getGridTile(patternLineRow, colm)->getColour() == tileColour) {
            validTurn = false;
            if(isLoading == false) {
                cout << "Tile is already within the grid row. Choose a different Tile or Row" << endl;
            }
        }
    }

    return validTurn;
}

void GameEngine::addTilesToMosaicFromPatternLine(Player* currentPlayer) {
    //create template for grid tiles to match
    Tile* matchGrid[ROWS][COLS] = 
     { {new Tile(DARK_BLUE), new Tile(YELLOW), new Tile(RED), new Tile(BLACK), new Tile(LIGHT_BLUE)},
        {new Tile(LIGHT_BLUE), new Tile(DARK_BLUE), new Tile(YELLOW), new Tile(RED), new Tile(BLACK)},
        {new Tile(BLACK), new Tile(LIGHT_BLUE), new Tile(DARK_BLUE), new Tile(YELLOW), new Tile(RED)},
        {new Tile(RED), new Tile(BLACK), new Tile(LIGHT_BLUE), new Tile(DARK_BLUE), new Tile(YELLOW)},
        {new Tile(YELLOW), new Tile(RED), new Tile(BLACK), new Tile(LIGHT_BLUE), new Tile(DARK_BLUE)}
     };
    
    //loops through each row of patternLine and matchGrid
     for(int row = 0; row < ROWS; ++row) {
        //create 1D array for each patternLine row
        Tile** patternLineRow = currentPlayer->getMosaic()->getPatternLineRow(row);
         //checks if patternLine is full
        if(currentPlayer->getMosaic()->patternLineFull(row)){
            //loops through each element of patternLine 
            for(int colm = 0; colm < COLS; ++colm) {
                //loops through each element of matchGrid 
                for(int i = 0; i < COLS; ++i) {
                    //compares if tile in element of patternLine matches matchGrid tile
                    if(patternLineRow[colm]->getCharColour() == matchGrid[row][i]->getCharColour()){
                        //adds tile of patternLine to grid
                        currentPlayer->getMosaic()->setGrid(new Tile(patternLineRow[colm]->getColour()),row, i);
                        //replace tile of patternLine with no tile
                        patternLineRow[colm] = new Tile(NO_TILE);

            }
            }
        }   
    }
}

    for(int row = 0; row < ROWS; ++row) {
         //create 1D array for each patternLine row
        Tile** patternLineRow = currentPlayer->getMosaic()->getPatternLineRow(row);
            //loops through each element of patternLine 
            for(int colm = 0; colm < COLS; ++colm) {
                //checks if there is a tile in element if patternLine
                if(patternLineRow[colm]->getCharColour() != NO_TILE && patternLineRow[colm]->getCharColour() != BLANK){
                    //adds tile back to tilebag
                    tileBag->addTile(new Tile(patternLineRow[colm]->getColour()));
                    //replace tile of patternLine with no tile
                    patternLineRow[colm] = new Tile(NO_TILE);
                }
            }
         }
}


void GameEngine::saveGame(string fileName) {
    // Create a new file with name defined by 'fileName' var
    ofstream saveFile;
    string allTurns;
    TileBag tileBag;

    // loop through the turns array and add each turn to a formatted string to save
    for(size_t n = 0; n < turns.size(); ++n) {
        allTurns.append(turns[n] + "\n");
    }
    // Add a .save extension to the file and save it in the 'saves' folder.
    saveFile.open(".//saves//" + fileName + ".save");

    // Write to this new file we created.
    // saveFile << tileBag.generateFixedTileBag() + "\n";
    saveFile << initTileBag + "\n";
    
    for(int i = 0; i < TOTAL_PLAYERS; ++i) {
        saveFile << players[i]->getPlayerName() << "\n";
    }
    
    saveFile << allTurns;
}


void GameEngine::printPlayerMosaic(Player* player) {
    Mosaic* mosaic = player->getMosaic();
    vector<Tile*> brokenTile = player->getMosaic()->getBrokenTiles();

    for(int row = 0; row < ROWS; ++row) {
        std::cout << row + 1 << " ";

        //Print PatternLine
        Tile** patternLineRow = mosaic->getPatternLineRow(row);
        for(int colm = 0; colm < COLS; ++colm) {
            cout << patternLineRow[colm]->getCharColour() << " ";
        }

        std::cout << "|| ";

        //Print Grid
        for(int colm = 0; colm < COLS; ++colm) {
            cout << mosaic->getGridTile(row, colm)->getCharColour() << " ";
        }

        std::cout << endl;
    }

    //prints broken tile by looping through vector
    std::cout << "broken: " ;
    for(int i = 0; i < (signed int) brokenTile.size(); ++i){
        std::cout << brokenTile.at(i)->getCharColour() << " ";
    }
    
    std::cout << std::endl;
}

int GameEngine::calculatePlayerScores(Player* player) {
    int roundScore = 0;

    //Iterate through entire grid and check for non- 'NO-TILE' tiles 
    for (int row = 0; row != ROWS; ++row){
        for (int colm = 0; colm != COLS; ++colm) {
            Mosaic* playerMosaic = player->getMosaic();

            if(playerMosaic->getGridTile(row, colm)->getColour() != NO_TILE) {
                //Score point for individual tile
                ++roundScore;

                //Score additional point for an intersecting tile:
                //Check if there is tile above or below
                if((colm - 1 >= 0 && playerMosaic->getGridTile(row, colm - 1)->getColour() != NO_TILE) ||
                    (colm + 1 < COLS && playerMosaic->getGridTile(row, colm + 1)->getColour() != NO_TILE)) {
                    
                    //Check if there is tile right or left
                    if((row - 1 >= 0 && playerMosaic->getGridTile(row - 1, colm)->getColour() != NO_TILE) ||
                        (row + 1 < ROWS && playerMosaic->getGridTile(row + 1, colm)->getColour() != NO_TILE)) {
                            ++roundScore;
                        }
                }
            }
        }
    }



   vector<Tile*> brokenTiles = player->getMosaic()->getBrokenTiles();
    for(int i=0; i<brokenTiles.size() ; ++i){
        if(i<2){
            --roundScore;
        }

        else if(i >= 2 && i<4){
            roundScore -=2;
        }

        else if(i >= 4 ){
            roundScore -=3;
        }
    }

    player->setPlayerScore(roundScore);
    
    return roundScore;
}

void GameEngine::setGameVariables(string player1, string player2, string newTileBag, vector <string> allTurns) {
    //Set the new tile bag from the save file being loaded.
    initTileBag = tileBag->generateTileBag(newTileBag);

    // //Create new players, with the names provided by the save file.
    // Player* player_1 = new Player(player1);
    // Player* player_2 = new Player(player2);
    
    //TBD: Loop through the vector of turns and perform them all? Would need a way of getting 
    // currentPlayer passed through. (maybe just runGame()?)
}
