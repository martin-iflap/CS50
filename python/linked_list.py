class Node:
    def __init__(self, value = None):
        self.value = value
        self.next = None

class LinkedList:
    def __init__(self):
        self.head = Node()

    def prepend(self, value):
        new_node = Node(value)
        new_node.next = self.head.next
        self.head.next = new_node

    def append(self, value):
        new_node = Node(value)
        current = self.head
        while current.next is not None:
            current = current.next
        current.next = new_node

    def display(self):
        """Print all the elements of the linked list"""
        elements = []
        current = self.head
        while current.next is not None:
            current = current.next
            elements.append(current.value)
        return elements

    def length(self):
        leng = 0
        current = self.head
        while current.next is not None:
            leng += 1
            current = current.next
        return leng

    def get(self, index):
        """Get the value of a node at a specific index"""
        if index >= self.length() or index < 0:
            return None
        current = self.head
        for n in range(index + 1):
            current = current.next
        return current.value

    def delete(self, index):
        if index < 0:
            return
        prev = self.head
        i = 0
        while prev.next is not None and i < index:
            prev = prev.next
            i += 1
        if prev.next is None:
            return
        to_remove = prev.next
        prev.next = to_remove.next
        to_remove.next = None

    def clear(self):
        self.head.next = None

    def reverse(self):
        prev = None
        current = self.head.next
        next_link = None
        while current is not None:
            next_link = current.next
            current.next = prev
            prev = current
            current = next_link
        self.head.next = prev

if __name__ == "__main__":
    ll = LinkedList()
    ll.append(1)
    ll.append(2)
    ll.prepend(0)
    print(ll.display())
    ll.delete(1)
    print(ll.display())
    ll.reverse()
    print(ll.display())
