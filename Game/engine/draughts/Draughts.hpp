#pragma once

#include "Object.hpp"

#include <array>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <thread>

#include "Board.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Draught.hpp"
#include "Object.hpp"

inline Draught::Color operator !(Draught::Color color)
{
    return color == Draught::Color::White ? Draught::Color::Black : Draught::Color::White;
}

using BoardMatrix = std::array<std::array<Draught*, 10>, 10>;

class Draughts
{
    struct Move
    {
        Draught* draught;
        std::vector<Draught::Position> moves;
        std::vector<Draught*> captured;
        int32_t maxNetCaptureWin;
    };

    class Ai
    {
    public:
        Ai(Draught::Color color)
            : _color(color)
        {
            // Initialize random number generator
            _rng = std::mt19937(_dev());
            _dist = std::uniform_int_distribution<std::mt19937::result_type>(0, 1000);
        }
        
        std::shared_ptr<std::future<Move>> future()
        {
            return _future;
        }
        
        void reset()
        {
            _future = nullptr;
        }
        
        void findOptimalMoveAsync(BoardMatrix boardMatrix, std::vector<Move> allPossibleMoves)
        {
            _future = std::make_shared<std::future<Move>>(std::async(std::launch::async, &Ai::findOptimalMove, this, boardMatrix, allPossibleMoves));
        }
        
    private:
        Move findOptimalMove(BoardMatrix boardMatrix, std::vector<Move> allPossibleMoves)
        {
            _pathsWandered = 0;
            int32_t maxNetCaptureWin = 0;
            for (auto&& possibleMove : allPossibleMoves) {
                possibleMove.maxNetCaptureWin = checkNetCaptureWin(possibleMove, boardMatrix, !_color, 3);
                maxNetCaptureWin = std::max(maxNetCaptureWin, possibleMove.maxNetCaptureWin);
            }
            
            // Remove the less optimal moves
            //allPossibleMoves.erase(std::remove_if(allPossibleMoves.begin(), allPossibleMoves.end(), [&maxNetCaptureWin](auto& possibleMove) {
            //    return possibleMove.maxNetCaptureWin < maxNetCaptureWin;
            //}), allPossibleMoves.end());

            // Select random move from the remaining equal possibilities
            uint32_t selectedMove = _dist(_rng) % allPossibleMoves.size();

            std::cout << "Amount of paths wandered: " << _pathsWandered << std::endl;
            return allPossibleMoves.at(selectedMove);
        }
        
        /**
         * Calculates recursively the amount of relative captures a move would make in it's depth.
         * A positive number means a relative capture win for white, a negative for black.
         */
        int32_t checkNetCaptureWin(Move move, BoardMatrix boardMatrix, Draught::Color turn, uint8_t depth)
        {
            _pathsWandered++;
            auto oldPosition = move.draught->position();
            boardMatrix[oldPosition.y][oldPosition.x] = nullptr;
            auto newPosition = move.moves[move.moves.size()-1];
            boardMatrix[newPosition.y][newPosition.x] = move.draught;
            
            for (auto& captured : move.captured) {
                auto capturedPosition = captured->position();
                boardMatrix[capturedPosition.y][capturedPosition.x] = nullptr;
            }
            
            std::vector<Move> allPossibleMoves;
            for (int index = 0; index < 100; index++) {
                auto draught = boardMatrix[index/10][index%10];
                if (draught == nullptr || draught->color() != turn) {
                    continue;
                }
                
                auto possibleMoves = findMoves(boardMatrix, draught, draught->position(), {});
                allPossibleMoves.insert(allPossibleMoves.end(), possibleMoves.begin(), possibleMoves.end());
            }
            
            if (allPossibleMoves.empty()) {
                return 0;
            }
            
            auto maxCapture = std::max_element(allPossibleMoves.begin(), allPossibleMoves.end(), [](auto& a, auto& b) {
                return a.captured.size() < b.captured.size();
            })->captured.size();
            
            allPossibleMoves.erase(std::remove_if(allPossibleMoves.begin(), allPossibleMoves.end(), [&maxCapture](auto& possibleMove) {
                return possibleMove.captured.size() < maxCapture;
            }), allPossibleMoves.end());
            
            int32_t maxNetCaptureWin = 0;
            for (auto&& possibleMove : allPossibleMoves) {
                int32_t netCaptureWin = static_cast<int32_t>(possibleMove.captured.size());
                if (_color != turn) {
                    netCaptureWin *= -1;
                }
                
                if (depth > 0) {
                    netCaptureWin += checkNetCaptureWin(possibleMove,
                                                        boardMatrix,
                                                        !turn,
                                                        depth - 1);
                }
                
                if (_color == turn) {
                    maxNetCaptureWin = std::max(maxNetCaptureWin, netCaptureWin);
                } else {
                    maxNetCaptureWin = std::min(maxNetCaptureWin, netCaptureWin);
                }
            }
            
            return maxNetCaptureWin;
        }
        
    private:
        const Draught::Color _color;

        bool _calculating{false};
        std::shared_ptr<std::future<Move>> _future;
        
        // Debugging of amount of nodes in AI branches
        int32_t _pathsWandered;
        
        std::random_device _dev;
        std::mt19937 _rng;
        std::uniform_int_distribution<std::mt19937::result_type> _dist;
    };
    
public:
    Draughts(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool)
        : _ai(Draught::Color::Black)
    {
        
        _boardObject = std::make_shared<Object>(device, commandPool, "objects/draughts_board.obj", 3, 1);
        _board = std::make_shared<Board>(_boardObject->instanceData(0));
        
        _draughtsObject = std::make_shared<Object>(device, commandPool, "objects/draught.obj", 4, 40);
        
        for (int i = 0; i < 40; ++i) {
            int8_t x = i < 20 ? i / 5
                              : 9 - ((i - 20) / 5);
            int8_t y = i < 20 ? (((i / 5) % 2 == 0) ? 1 : 0) + (i % 5) * 2
                              : (((i / 5) % 2 == 0) ? 8 : 9) - (i % 5) * 2;
            Draught::Color color = i < 20 ? Draught::Color::White
                                          : Draught::Color::Black;
            _draughts.emplace_back(_draughtsObject->instanceData(i), Draught::Position{x, y}, color);
        }
    }
    
    std::shared_ptr<Object> boardObject()
    {
        return _boardObject;
    }
    
    std::shared_ptr<Object> draughtsObject()
    {
        return _draughtsObject;
    }
    
    void move(std::optional<Draught::Position> playerMove = std::nullopt)
    {
        BoardMatrix boardMatrix;
        for (int i = 0; i < 100; ++i) {
            boardMatrix[i/10][i%10] = nullptr;
        }
        
        std::for_each(_draughts.begin(), _draughts.end(), [this, &boardMatrix](Draught& draught) {
            if (draught.state() != Draught::State::Captured && draught.state() != Draught::State::Crown) {
                auto position = draught.position();
                boardMatrix[position.y][position.x] = &draught;
            }
        });
        
        std::vector<Move> allPossibleMoves;
        for (int index = 0; index < 100; index++) {
            auto draught = boardMatrix[index/10][index%10];
            if (draught == nullptr || draught->color() != _turn) {
                continue;
            }
            
            auto possibleMoves = findMoves(boardMatrix, draught, draught->position(), {});
            allPossibleMoves.insert(allPossibleMoves.end(), possibleMoves.begin(), possibleMoves.end());
        }
        
        // Nothing possible anymore! TODO: Handle win.
        if (allPossibleMoves.empty()) {
            std::cout << (_turn == Draught::Color::White ? "White" : "Black") << " is the winner!..." << std::endl;
            return;
        }
        
        auto maxCapture = std::max_element(allPossibleMoves.begin(), allPossibleMoves.end(), [](auto& a, auto& b) {
            return a.captured.size() < b.captured.size();
        })->captured.size();
        
        allPossibleMoves.erase(std::remove_if(allPossibleMoves.begin(), allPossibleMoves.end(), [&maxCapture](auto& possibleMove) {
            return possibleMove.captured.size() < maxCapture;
        }), allPossibleMoves.end());
        
        Move move;
        if (playerMove) {
            _board->addSelected(playerMove->x, playerMove->y);
            for (auto&& draught : _draughts) {
                if (draught.selected()) {
                    move.draught = &draught;
                }
            }
            
            for (auto&& possibleMove : allPossibleMoves) {
                if (move.draught != possibleMove.draught) {
                    continue;
                }
                
                bool previousMovesAreLegit = true;
                for (int i = 0; i < _playerMoves.size(); ++i) {
                    if (possibleMove.moves[i] != _playerMoves[i]) {
                        previousMovesAreLegit = false;
                        break;
                    }
                }
                
                if (!previousMovesAreLegit || possibleMove.moves[_playerMoves.size()] != *playerMove) {
                    continue;
                }
                
                _playerMoves.emplace_back(*playerMove);
                if (_playerMoves.size() == possibleMove.moves.size()) {
                    move.moves = _playerMoves;
                    move.captured = possibleMove.captured;
                    break;
                } else {
                    // Move not finished yet
                    return;
                }
            }
            
            if (move.moves.empty()) {
                std::cerr << "Move not allowed, resetting move queue and selection." << std::endl;
                
                _playerMoves.clear();
                _board->resetSelected();
                std::for_each(_draughts.begin(), _draughts.end(), [](auto&& draught){
                    draught.setSelected(false);
                });
                return;
            }
            
            // Reset queue for next move
            _playerMoves.clear();
        } else if (_ai.future() && _ai.future()->valid()) {
            if (_ai.future()->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                move = _ai.future()->get();
                _ai.reset();
            } else {
                // Continue idling if AI is not done yet
                return;
            }
        } else {
            // Start async check for most optimal move
            _ai.findOptimalMoveAsync(boardMatrix, allPossibleMoves);
            return;
        }

        move.draught->setPosition(move.moves);
        if (move.draught->state() != Draught::State::Lady) {
            if ((_turn == Draught::Color::White && move.draught->position().x == 9)
             || (_turn == Draught::Color::Black && move.draught->position().x == 0)) {
                move.draught->setState(Draught::State::Lady);
                
                // Make the last captured piece the crown
                for (auto&& draught : _draughts) {
                    if (_turn == Draught::Color::White) {
                        if (draught.id() == _capturedWhiteIds[_capturedWhiteIds.size()-1]) {
                            move.draught->setCrown(&draught);
                            _capturedWhiteIds.erase(std::prev(_capturedWhiteIds.end()));
                        }
                    } else {
                        if (draught.id() == _capturedBlackIds[_capturedBlackIds.size()-1]) {
                            move.draught->setCrown(&draught);
                            _capturedBlackIds.erase(std::prev(_capturedBlackIds.end()));
                        }
                    }
                }
                
                // Put the crown on the lady only once she reached the end
                std::vector<Draught::Position> crownMoves;
                for (int i = 0; i < move.moves.size(); ++i) {
                    crownMoves.emplace_back(move.draught->crown()->position());
                }
                crownMoves.emplace_back(move.moves[move.moves.size()-1]);
                move.draught->crown()->setPosition(crownMoves);
            }
        }
        
        for (uint32_t i = 0; i < move.captured.size(); ++i) {
            std::vector<Draught::Position> moves;
            moves.emplace_back(move.captured[i]->position());
            for (int j = 0; j < i; j++) {
                moves.emplace_back(move.captured[i]->position());
            }
            
            int8_t x;
            int8_t y;
            if (_turn == Draught::Color::White) {
                x = _capturedBlackIds.size() % 5;
                y = -1 - _capturedBlackIds.size() / 5;
                _capturedBlackIds.emplace_back(move.captured[i]->id());
            } else {
                x = 9 - (_capturedWhiteIds.size() % 5);
                y = 10 + _capturedWhiteIds.size() / 5;
                _capturedWhiteIds.emplace_back(move.captured[i]->id());
            }
            
            moves.emplace_back(Draught::Position{x, y});
            move.captured[i]->setPosition(moves);
            move.captured[i]->setState(Draught::State::Captured);
        }
        
        _turn = !_turn;
    }
    
    void update()
    {
        bool animating = false;
        std::for_each(_draughts.begin(), _draughts.end(), [&animating](auto& draught) {
            animating |= draught.animate();
        });
        
        if (!animating && _turn == Draught::Color::Black) {
            // Reset selection once computer start moving
            _board->resetSelected();
            std::for_each(_draughts.begin(), _draughts.end(), [](auto& draught) {
                draught.setSelected(false);
            });
            
            move();
        }
    }
    
    std::vector<Draught> draughts()
    {
        return _draughts;
    }
    
private:
    static std::vector<Move> findMoves(BoardMatrix boardMatrix, Draught* draught, Draught::Position position, std::vector<int8_t> capturedIds)
    {
        std::vector<Move> possibleMoves;
        bool lady = draught->state() == Draught::State::Lady;
        Draught::Color turn = draught->color();
        
        for (int8_t i = 0; i < 4; ++i) {
            int8_t dX = (i / 2) * 2 - 1;
            int8_t dY = (((i+1) / 2) % 2) * 2 - 1;
            
            int8_t depth = lady ? 10 : 3;
            for (int8_t j = 1; j < depth; ++j) {
                int8_t x = position.x + dX * j;
                int8_t y = position.y + dY * j;
                
                // Don't jump off the board, doesn't make sense to search further
                if (x < 0 || x > 9 || y < 0 || y > 9) {
                    break;
                }
                
                // Need to land on a free field in the end
                if (boardMatrix[y][x] != nullptr) {
                    continue;
                }
                
                bool isValidMove = true;
                Draught* captured = nullptr;
                for (int8_t k = 1; k < j; ++k) {
                    int8_t xPath = position.x + dX * k;
                    int8_t yPath = position.y + dY * k;
                    if (boardMatrix[yPath][xPath] != nullptr) {
                        if (turn == boardMatrix[yPath][xPath]->color()) {
                            // Jumping over own pieces not allowed
                            isValidMove = false;
                            break;
                        }
                        
                        if (captured != nullptr) {
                            // Jumping over two at the same time not possible
                            isValidMove = false;
                            continue;
                        }
                        
                        // Cannot capture which is already captured
                        if (std::find(capturedIds.begin(), capturedIds.end(), boardMatrix[yPath][xPath]->id()) != capturedIds.end()) {
                            break;
                        }
                        
                        // Jumping over one is allowed and encouraged
                        captured = boardMatrix[yPath][xPath];
                    }
                }
                        
                if (!isValidMove) {
                    break;
                }
                
                // Moving without capturing is only possible on the first move
                if (captured == nullptr && !capturedIds.empty()) {
                    break;
                }
                
                if (captured == nullptr && !lady) {
                    if (j > 1) {
                        // Only ladies can jump more than one field without capturing
                        break;
                    } else if ((turn == Draught::Color::White && dX < 0) || (turn == Draught::Color::Black && dX > 0)) {
                        // Only ladies can jump back without capturing
                        break;
                    }
                }
                
                Draught::Position nextPosition{x, y};
                
                Move possibleMove;
                possibleMove.draught = draught;
                possibleMove.moves.emplace_back(nextPosition);
                
                // No followup moves possible without capture
                if (captured == nullptr) {
                    possibleMoves.emplace_back(possibleMove);
                    continue;
                }
                
                possibleMove.captured.emplace_back(captured);
                
                // Copy by value to keep the original values
                auto nextCapturedIds = capturedIds;
                nextCapturedIds.emplace_back(captured->id());
                
                BoardMatrix nextBoardMatrix = boardMatrix;
                nextBoardMatrix[y][x] = draught;
                nextBoardMatrix[captured->position().y][captured->position().x] = nullptr;
                auto nextPossibleMoves = findMoves(nextBoardMatrix, draught, nextPosition, nextCapturedIds);
                        
                // If no further moves are possible, at least save this capture in the list of possible moves
                if (nextPossibleMoves.empty()) {
                    possibleMoves.emplace_back(possibleMove);
                    continue;
                }
                        
                // Otherwise, use the current move as the base to add the list of the other moves on top of it
                for (const auto& nextPossibleMove : nextPossibleMoves) {
                    Move move = possibleMove;
                    move.moves.insert(move.moves.end(), nextPossibleMove.moves.begin(), nextPossibleMove.moves.end());
                    move.captured.insert(move.captured.end(), nextPossibleMove.captured.begin(), nextPossibleMove.captured.end());
                    
                    possibleMoves.emplace_back(move);
                }
            }
        }
        
        return possibleMoves;
    }
    
private:
    Draught::Color _turn{Draught::Color::White};
    std::vector<Draught::Position> _playerMoves;
    
    std::shared_ptr<Object> _boardObject;
    std::shared_ptr<Board> _board;
    
    std::shared_ptr<Object> _draughtsObject;
    std::vector<Draught> _draughts;
    
    std::vector<int8_t> _capturedWhiteIds;
    std::vector<int8_t> _capturedBlackIds;
    
    Ai _ai;
};
