#include "BlockStore/common/core.h"

#include <string>
#include <vector>
#include <iostream>


using namespace BlockStore;


template<class T, class = std::enable_if<!std::is_pointer_v<T> && !std::is_reference_v<T>>>
class BlockRef {
	BlockManager& manager;
	size_t index;
	std::unique_ptr<T> resource;
private:
	void Load() {
		if (resource == nullptr) {
			resource = manager.Load<T>(index);
		}
	}
public:
	T& operator*() { return resource.operator*(); }
};


template<class... Types>
struct Tuple {

};

template<class... Types>
struct Union {

};

template<class T>
struct List {

};

template<class T, size_t count>
struct Array {

};

template<class T>
struct Ref {

};


template<class StorageType, template<class ContainerType, class BlockType> ContainerType BlockType::*member>
struct Bind {};


struct TreeNode {
	std::string text;
	std::vector<BlockRef<TreeNode>> child_list;
};


template<class T>
struct LayoutHelper {
	using Layout = T::Layout;
};

template<>
struct LayoutHelper<TreeNode> {
	using Layout = Tuple<
		Bind<List<char>, &TreeNode::text>, 
		Bind<List<Ref<TreeNode>>, &TreeNode::child_list>
	>;
};

template<class T>
using Layout = typename LayoutHelper<T>::Layout;



void Load(TreeNode& node) {

}


struct Root {
	BlockRef<TreeNode> root_node;
};


void PrintNode(TreeNode& node, uint depth) {
	static constexpr uint max_depth = 127;
	static const std::string tab_padding(max_depth, '\t');
	static const std::string_view tab_padding_view = tab_padding;
	static auto indent = [](uint level) { return tab_padding_view.substr(0, level); };

	std::cout << indent(depth) << node.text << std::endl;
	for (auto& node_ref : node.child_list) {
		PrintNode(*node_ref, depth + 1);
	}
}


int main() {
	


	TreeNode root = Global.OpenFile(L"");

	PrintNode(root, 0);
}