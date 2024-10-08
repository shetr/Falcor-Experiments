import DensityNode;

ParameterBlock<DensityNodes> gNodes;
RWByteAddressBuffer gNodesSize;

cbuffer CB
{
    float gSplittingThreshold;
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint nodeIndex = threadId.x;

    uint nodesSize = gNodesSize.Load<uint>(0);
    if (gNodes.isNodeRoot(nodeIndex) || nodeIndex >= nodesSize)
    {
        return;
    }

    for (int childIndex = 0; childIndex < 8; childIndex++)
    {
        if (nodesSize >= MAX_NODES_SIZE)
        {
            return;
        }

        if (!gNodes.isChildNodeLeaf(nodeIndex, childIndex))
        {
            continue;
        }

        uint depth = 0;
        uint parentOffset = childIndex;
        uint parentIndex = nodeIndex;
        float density = gNodes.getChildDensity(nodeIndex, childIndex);
        do
        {
            parentOffset = gNodes.getParentNodeOffset(parentIndex);
            parentIndex = gNodes.getParentNodeIndex(parentIndex);
            density *= gNodes.getChildDensity(parentIndex, parentOffset);
            depth += 1;
        } while (!gNodes.isNodeRoot(parentIndex));

        if (density <= gSplittingThreshold || depth >= MAX_OCTREE_DEPTH)
        {
            continue;
        }
        uint newNodeIndex;
        gNodesSize.InterlockedAdd(0, 1, newNodeIndex);
        if (newNodeIndex < MAX_NODES_SIZE)
        {
            float accumulator = gNodes.getChildAccumulator(nodeIndex, childIndex);
            gNodes.setChildNodeIndex(nodeIndex, childIndex, newNodeIndex);
            gNodes.setParentNodeIndex(newNodeIndex, nodeIndex);
            gNodes.setParentNodeOffset(newNodeIndex, childIndex);
            for (int ch = 0; ch < 8; ch++)
            {
                gNodes.setChildNodeIndex(newNodeIndex, ch, 0);
                gNodes.setChildAccumulator(newNodeIndex, ch, accumulator / 8.0);
                gNodes.setChildDensity(newNodeIndex, ch, 1.0 / 8.0);
            }
        }
        else
        {
            gNodesSize.Store(0, MAX_NODES_SIZE);
        }
    }
}
