# multiAgents.py
# --------------
# Licensing Information:  You are free to use or extend these projects for
# educational purposes provided that (1) you do not distribute or publish
# solutions, (2) you retain this notice, and (3) you provide clear
# attribution to UC Berkeley, including a link to http://ai.berkeley.edu.
# 
# Attribution Information: The Pacman AI projects were developed at UC Berkeley.
# The core projects and autograders were primarily created by John DeNero
# (denero@cs.berkeley.edu) and Dan Klein (klein@cs.berkeley.edu).
# Student side autograding was added by Brad Miller, Nick Hay, and
# Pieter Abbeel (pabbeel@cs.berkeley.edu).


from util import manhattanDistance
from game import Directions
import random, util

from game import Agent

class ReflexAgent(Agent):
    """
    A reflex agent chooses an action at each choice point by examining
    its alternatives via a state evaluation function.

    The code below is provided as a guide.  You are welcome to change
    it in any way you see fit, so long as you don't touch our method
    headers.
    """


    def getAction(self, gameState):
        """
        You do not need to change this method, but you're welcome to.

        getAction chooses among the best options according to the evaluation function.

        Just like in the previous project, getAction takes a GameState and returns
        some Directions.X for some X in the set {NORTH, SOUTH, WEST, EAST, STOP}
        """
        # Collect legal moves and successor states
        legalMoves = gameState.getLegalActions()

        # Choose one of the best actions
        scores = [self.evaluationFunction(gameState, action) for action in legalMoves]
        bestScore = max(scores)
        bestIndices = [index for index in range(len(scores)) if scores[index] == bestScore]
        chosenIndex = random.choice(bestIndices) # Pick randomly among the best

        "Add more of your code here if you want to"

        return legalMoves[chosenIndex]

    def evaluationFunction(self, currentGameState, action):
        """
        Design a better evaluation function here.

        The evaluation function takes in the current and proposed successor
        GameStates (pacman.py) and returns a number, where higher numbers are better.

        The code below extracts some useful information from the state, like the
        remaining food (newFood) and Pacman position after moving (newPos).
        newScaredTimes holds the number of moves that each ghost will remain
        scared because of Pacman having eaten a power pellet.

        Print out these variables to see what you're getting, then combine them
        to create a masterful evaluation function.
        """
        # Useful information you can extract from a GameState (pacman.py)
        successorGameState = currentGameState.generatePacmanSuccessor(action)
        newPos = successorGameState.getPacmanPosition()
        newFood = successorGameState.getFood()
        newGhostStates = successorGameState.getGhostStates()
        newScaredTimes = [ghostState.scaredTimer for ghostState in newGhostStates]

        "*** YOUR CODE HERE ***"
        score = 0
        oldFood = currentGameState.getFood()
        for ghostState in newGhostStates:
            dis = util.manhattanDistance(newPos, ghostState.getPosition())
            score -= 3/dis if dis != 0 else 15
        distances = []
        for x in range(oldFood.width):
            for y in range(oldFood.height):
                if oldFood[x][y]:
                    md = manhattanDistance(newPos, (x,y))
                    distances.append(md)
        minDis = min(distances)
        if minDis == 0:
            score += 2
        else:
            score += 1/minDis
        return score

def scoreEvaluationFunction(currentGameState):
    """
    This default evaluation function just returns the score of the state.
    The score is the same one displayed in the Pacman GUI.

    This evaluation function is meant for use with adversarial search agents
    (not reflex agents).
    """
    return currentGameState.getScore()

class MultiAgentSearchAgent(Agent):
    """
    This class provides some common elements to all of your
    multi-agent searchers.  Any methods defined here will be available
    to the MinimaxPacmanAgent, AlphaBetaPacmanAgent & ExpectimaxPacmanAgent.

    You *do not* need to make any changes here, but you can if you want to
    add functionality to all your adversarial search agents.  Please do not
    remove anything, however.

    Note: this is an abstract class: one that should not be instantiated.  It's
    only partially specified, and designed to be extended.  Agent (game.py)
    is another abstract class.
    """

    def __init__(self, evalFn = 'scoreEvaluationFunction', depth = '2'):
        self.index = 0 # Pacman is always agent index 0
        self.evaluationFunction = util.lookup(evalFn, globals())
        self.depth = int(depth)

class MinimaxAgent(MultiAgentSearchAgent):
    """
    Your minimax agent (question 2)
    """

    def getAction(self, gameState):
        """
        Returns the minimax action from the current gameState using self.depth
        and self.evaluationFunction.

        Here are some method calls that might be useful when implementing minimax.

        gameState.getLegalActions(agentIndex):
        Returns a list of legal actions for an agent
        agentIndex=0 means Pacman, ghosts are >= 1

        gameState.generateSuccessor(agentIndex, action):
        Returns the successor game state after an agent takes an action

        gameState.getNumAgents():
        Returns the total number of agents in the game

        gameState.isWin():
        Returns whether or not the game state is a winning state

        gameState.isLose():
        Returns whether or not the game state is a losing state
        """
        "*** YOUR CODE HERE ***"
        return self.minimax(gameState, self.depth, 0)[1]
        util.raiseNotDefined()

#Need to extract optimal action instead of just the value. How does the computation work then
#instead of returning the computing the value in depth n-1 return the action and compute at depth n
    def minimax(self, gameState, depth, agentIndex):
        if depth == 0 or gameState.isWin() or gameState.isLose():
            return (self.evaluationFunction(gameState), None)
        if agentIndex == 0: #maximizer (pacman)
            value = float('-inf')
            bestAction = None
            legalActions = gameState.getLegalActions(agentIndex)
            for action in legalActions:
                successor = gameState.generateSuccessor(agentIndex, action)
                minVal, minAction = self.minimax(successor,depth,agentIndex+1)
                if minVal > value:
                    value = minVal 
                    bestAction = action
            return (value, bestAction) 
        if agentIndex > 0: #minimizer (ghost)
            value = float('inf')
            legalActions = gameState.getLegalActions(agentIndex)
            for action in legalActions:
                successor = gameState.generateSuccessor(agentIndex, action)
                if agentIndex == gameState.getNumAgents()-1:
                    maxVal, maxAction = self.minimax(successor, depth-1, 0)
                else:
                    maxVal, maxAction = self.minimax(successor, depth, agentIndex+1)
                if maxVal < value:
                    value = maxVal
                    bestAction = action
            return (value, bestAction) 

class AlphaBetaAgent(MultiAgentSearchAgent):
    """
    Your minimax agent with alpha-beta pruning (question 3)
    """

    def getAction(self, gameState):
        """
        Returns the minimax action using self.depth and self.evaluationFunction
        """
        "*** YOUR CODE HERE ***"
        return self.AB(gameState, self.depth, float('-inf'), float('inf'), 0)[1]

    def AB(self, gameState, depth, alpha, beta, agentIndex):
        if depth == 0 or gameState.isWin() or gameState.isLose():
            return (self.evaluationFunction(gameState), None)
        if agentIndex == 0:
            bestAction = None
            value = float('-inf')
            legalActions = gameState.getLegalActions(agentIndex)
            for action in legalActions: 
                successor = gameState.generateSuccessor(agentIndex, action)
                minVal, minAction = self.AB(successor, depth, alpha, beta, agentIndex+1)
                if minVal > value:
                    value = minVal
                    bestAction = action
                if value > beta:
                    break
                alpha = max(alpha, value)
            return (value, bestAction)
        if agentIndex > 0:
            value = float('inf')
            bestAction = None
            legalActions = gameState.getLegalActions(agentIndex)
            for action in legalActions:
                successor = gameState.generateSuccessor(agentIndex, action)
                if agentIndex == gameState.getNumAgents()-1:
                    maxVal, maxAction = self.AB(successor, depth-1, alpha, beta, 0)
                else:
                    maxVal, maxAction = self.AB(successor, depth, alpha, beta, agentIndex+1)
                if maxVal < value:
                    value = maxVal
                    bestAction = action
                if value < alpha:
                    break
                beta = min(value, beta)
            return (value, bestAction)
            
class ExpectimaxAgent(MultiAgentSearchAgent):
    """
      Your expectimax agent (question 4)
    """

    def getAction(self, gameState):
        """
        Returns the expectimax action using self.depth and self.evaluationFunction

        All ghosts should be modeled as choosing uniformly at random from their
        legal moves.
        """
        "*** YOUR CODE HERE ***"
        return self.expectimax(gameState, self.depth, 0)[1]
    
    def expectimax(self, gameState, depth, agentIndex):
        if depth == 0 or gameState.isWin() or gameState.isLose():
            return (self.evaluationFunction(gameState), None)
        if agentIndex == 0: #maximizer (pacman)
            value = float('-inf')
            bestAction = None
            legalActions = gameState.getLegalActions(agentIndex)
            for action in legalActions:
                successor = gameState.generateSuccessor(agentIndex, action)
                minVal, minAction = self.expectimax(successor,depth,agentIndex+1)
                if minVal > value:
                    value = minVal 
                    bestAction = action
            return (value, bestAction) 
        if agentIndex > 0: #minimizer (ghost)
            value = float('inf')
            legalActions = gameState.getLegalActions(agentIndex)
            expectedvalue = 0
            for action in legalActions:
                successor = gameState.generateSuccessor(agentIndex, action)
                if agentIndex == gameState.getNumAgents()-1:
                    maxVal, maxAction = self.expectimax(successor, depth-1, 0)
                else:
                    maxVal, maxAction = self.expectimax(successor, depth, agentIndex+1)
                expectedvalue += maxVal*(1/len(legalActions))
            return (expectedvalue, None) 

def betterEvaluationFunction(currentGameState):
    """
    Your extreme ghost-hunting, pellet-nabbing, food-gobbling, unstoppable
    evaluation function (question 5).

    DESCRIPTION: The evaluation function penalizing pacman for having remaining pellets and capsules. Capsules are weighted more significantly
    because eating a ghost is a major boost to the score. Addtionally Pacman is penalized for being near non-scared ghosts but is heavily incentivized
    to be close to scared ghosts. The score bonus from eating a ghost is reflected by the initialization of score to the current game state score which
    will be boosted by eating the ghost.
    """
    "*** YOUR CODE HERE ***"
    #score should increase for fewer food pellets
    #score should increase for fewer powerups
    #score should drastically increase for eating a ghost
    #score should decrease for getting close to a ghost
    pos = currentGameState.getPacmanPosition()
    score = currentGameState.getScore()
    score -= 20 * currentGameState.getNumFood()
    score -= 100 * len(currentGameState.getCapsules())
    for ghostState in currentGameState.getGhostStates():
        distance = manhattanDistance(pos, ghostState.getPosition())
        if ghostState.scaredTimer:
            score += 150*(1/(distance+1))
        else:
            score -= 10*(1/(distance+1))
    return score

    
# Abbreviation
better = betterEvaluationFunction
