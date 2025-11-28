class TreeNode:
    def __init__(self, value, data=None):
        self.value = value
        self.left = None
        self.right = None
        self.data = data # Additional data can be stored here

    def insert(self, value, data=None) -> None:
        """Insert a new value into the tree"""
        if value < self.value:
            if self.left is None:
                self.left = TreeNode(value, data)
            else:
                self.left.insert(value, data)
        else:
            if self.right is None:
                self.right = TreeNode(value, data)
            else:
                self.right.insert(value, data)

    def inorder_traversal(self) -> list:
        """Return the inorder traversal (smallest to largest) of the tree as a list"""
        elements = []
        if self.left:
            elements.extend(self.left.inorder_traversal())
        elements.append(self.value)
        if self.right:
            elements.extend(self.right.inorder_traversal())
        return elements

    def preorder_traversal(self) -> list:
        """Return the preorder traversal (print root -> left -> right for all subtrees) of the tree as a list"""
        elements = [self.value]
        if self.left:
            elements.extend(self.left.preorder_traversal())
        if self.right:
            elements.extend(self.right.preorder_traversal())
        return elements

    def postorder_traversal(self) -> list:
        """Return the postorder traversal (print left -> right -> root for all subtrees) of the tree as a list"""
        elements: list = []
        if self.left:
            elements.extend(self.left.postorder_traversal())
        if self.right:
            elements.extend(self.right.postorder_traversal())
        elements.append(self.value)
        return elements

    def search(self, value) -> bool:
        """Search for a value in the tree. Return True if found, else False"""
        if value < self.value:
            if self.left is None:
                return False
            return self.left.search(value)
        elif value == self.value:
            return self.data if self.data else True
        else:
            if self.right is None:
                return False
            return self.right.search(value)

    def print_tree(self, level=0):
        """Print the tree structure rotated 90 degrees to the left"""
        if self.right:
            self.right.print_tree(level + 1)
        print(' ' * 4 * level + '->', self.value)
        if self.left:
            self.left.print_tree(level + 1)

    def height(self):
        """Return the height of the tree"""
        if not self:
            return 0
        left_height = self.left.height() if self.left else 0
        right_height = self.right.height() if self.right else 0
        return 1 + max(left_height, right_height)

    def is_balanced(self) -> list[bool | int]:
        """Check if the tree is balanced"""
        def is_balanced_helper(root) -> list[bool | int]:
            """Helper function to check if the tree is balanced and also return its height"""
            if not root:
                return [True, 0]
            l_balance, r_balance = is_balanced_helper(root.left), is_balanced_helper(root.right)
            balance = l_balance[0] and r_balance[0] and abs(l_balance[1] - r_balance[1]) <= 1
            height = max(l_balance[1], r_balance[1]) + 1
            return [balance, height]
        return is_balanced_helper(self)

    def rebuild_rebalance(self) -> 'TreeNode':
        """Rebalance the tree by rebuilding it from its inorder traversal"""
        elements = self.inorder_traversal()
        def build_balanced_tree(elem) -> TreeNode:
            length = len(elem)
            mid = length // 2
            root = TreeNode(elem[mid])
            if mid > 0:
                root.left = build_balanced_tree(elem[:mid])
            if (mid + 1) < length:
                root.right = build_balanced_tree(elem[mid + 1:])
            return root
        return build_balanced_tree(elements)

    def rebalance_height(self) -> 'TreeNode':
        """Rebalance the tree using the heights of the nodes and rotations (AVL Tree style)
           but without storing height in the nodes"""
        def get_balance(node) -> int:
            """Get the balance factor of a node using the height function"""
            if not node:
                return 0
            l_height = node.left.height() if node.left else 0
            r_height = node.right.height() if node.right else 0
            return l_height - r_height

        def rotate_right(root) -> 'TreeNode':
            """Rotate the given tree right"""
            new = root.left
            root.left = new.right
            new.right = root
            return new

        def rotate_left(root) -> 'TreeNode':
            """Rotate the given tree left"""
            new = root.right
            root.right = new.left
            new.left = root
            return new

        def rebalance_height_helper(root) -> 'TreeNode':
            """Rebalance the tree recursively using rotations of subtrees"""
            if not root:
                return root

            root.left = rebalance_height_helper(root.left)
            root.right = rebalance_height_helper(root.right)
            balance = get_balance(root)

            if balance > 1:
                if root.left:
                    left_balance = get_balance(root.left)
                    if left_balance < 0:
                        root.left = rotate_left(root.left)
                root = rotate_right(root)

            elif balance < -1:
                if root.right:
                    right_balance = get_balance(root.right)
                    if right_balance > 0:
                        root.right = rotate_right(root.right)
                root = rotate_left(root)
            return root
        return rebalance_height_helper(self)


if __name__ == "__main__":
    a = "Additional data"
    tree = TreeNode(1)
    tree.insert(2)
    tree.insert(3, data=a)
    print(tree.search(3))
    print(tree.search(2))
