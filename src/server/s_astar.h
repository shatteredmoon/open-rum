#ifndef _S_ASTAR_H_
#define _S_ASTAR_H_

#include <u_map.h>
#include <u_structs.h>
#include <u_utility.h>

class rumPawn;


// This is our A* class for the pathfinder. A* works by sending out feeler nodes in 8 directions. Each feeler proceeds
// with its own 8 feelers. When contact is made, then it tries to find the shortest point back to the starting point.
//
// g - the actual shortest distance traveled from initial node to current node
// h - the estimated (or "heuristic") distance from current node to goal
// f - the sum of g(x) and h(x)

class rumAstarPath
{

public:

  ~rumAstarPath();

  // Builds a path from the start position to the destination position using the specified movement type
  bool FindNewPath( rumPawn* io_pcPawn, const rumPosition& i_rcPos, const rumMap* i_pcMap,
                    const uint32_t i_uiMaxTileDistance, const float i_fMaxCost, const rumDirectionType i_eDir );

  void ForgetPath()
  {
    FreePathNodes();
  }


  rumPosition GetPos() const
  {
    return ( HasPath() ? m_pcPathList->m_cPos : m_cOriginPos );
  }


  rumPosition GetTargetPos() const
  {
    return m_cTargetPos;
  }


  // If the PathList is non-null, the pawn is at the start, or somewhere along the path
  bool HasPath() const
  {
    return m_pcPathList != nullptr;
  }


  // Advance the path pointer
  void PathNextNode()
  {
    if( HasPath() )
    {
      m_pcPathList = m_pcPathList->m_pcParent;
    }
  }

private:

  // A path node represents one world tile
  struct rumPathNode
  {
    // The sum of g and h
    float m_fFScore{ 0.f };

    // Shortest distance traveled from initial node to current node
    float m_fGScore{ 0.f };

    // Estimated "heuristic" distance from current node to goal
    uint32_t m_uiHScore{ 0 };

    rumPosition m_cPos;

    rumPathNode* m_pcParent{ nullptr };
    rumPathNode* m_pcNext{ nullptr };
    rumPathNode* m_pcPrev{ nullptr };
  };

  // See if a Path Node at the specified position exists on the Closed List
  rumPathNode* CheckClosedPath( const rumPosition& i_rcPos );

  // See if a Path Node at the specified position exists on the Open List
  rumPathNode* CheckOpenPath( const rumPosition& i_rcPos );

  // Validate the position we want to move to and generate a path node for the position
  void CheckPosition( rumPathNode* io_pcBestNode, rumPosition& io_rcPos, const rumMap* i_pcMap, rumPawn* io_pcPawn,
                      const float i_fMaxMoveCost, const float i_fMoveCost );

  // The A* algorithm
  void FindPath( const rumPosition& i_rcPos, const rumMap* i_pcMap, rumPawn* io_pcPawn, const float i_fMaxCost,
                 const rumDirectionType i_eDir );

  // Deallocates all existing nodes on the open and closed lists
  void FreePathNodes();

  // The current position is a good candidate, so a Path Node should be created. Calculate the cost and make sure this
  // position does not already exist on the Open and Closed lists. Also, if this path is getting too long, stop looking
  // in this direction.
  void GenerateSuccessorNode( rumPathNode* i_pcParentNode, const rumPosition& i_rcPos, const float i_fMaxMoveCost,
                              const float i_fMoveCost );

  // Send out feelers in only the 4 cardinal directions, generating a node for valid locations
  void GenerateSuccessors4( rumPathNode* i_pcBestNode, const rumMap* i_pcMap, rumPawn* i_pcPawn,
                            const float i_fMaxMoveCost );

  // Send out feelers in all 8 directions, generating a node for valid locations
  void GenerateSuccessors8( rumPathNode* i_pcBestNode, const rumMap* i_pcMap, rumPawn* i_pcPawn,
                            const float i_fMaxMoveCost );

  // Find and return the Path Node with the lowest cost on the open list
  rumPathNode* GetBestNode();

  // Add the new PathNode to the open list so that it gets visited
  void InsertPathNode( rumPathNode* i_pcSuccessorNode );

  // List of Path Nodes that should be visited
  rumPathNode* m_pcOpenList{ nullptr };

  // List of Path Nodes already visited
  rumPathNode* m_pcClosedList{ nullptr };

  // The finished, optimal path
  rumPathNode* m_pcPathList{ nullptr };

  // Is pathfinding finished and a goal reached?
  bool m_bPathComplete{ false };

  // The origin position of the AI finding a path
  rumPosition m_cOriginPos;

  // The target position of the AI finding a path
  rumPosition m_cTargetPos;

  // The default maximum amount of tiles an AI can step across
  static constexpr int32_t s_iDefaultMaxTileDistance{ 11 };
};

#endif // _S_ASTAR_H_
