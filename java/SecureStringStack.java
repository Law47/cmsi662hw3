public final class SecureStringStack {
    private static final int DEFAULT_INITIAL_CAPACITY = 8;

    private String[] elements;
    private int size;

    public SecureStringStack() {
        this(DEFAULT_INITIAL_CAPACITY);
    }

    public SecureStringStack(int initialCapacity) {
        if (initialCapacity <= 0) {
            throw new IllegalArgumentException("initialCapacity must be > 0");
        }
        this.elements = new String[initialCapacity];
        this.size = 0;
    }

    public void push(String value) {
        if (value == null) {
            throw new IllegalArgumentException("value must not be null");
        }
        ensureCapacity(size + 1);
        elements[size] = new String(value);
        size++;
    }

    public String pop() {
        if (size == 0) {
            throw new IllegalStateException("cannot pop from empty stack");
        }
        size--;
        String value = elements[size];
        elements[size] = null;
        return new String(value);
    }

    public String peek() {
        if (size == 0) {
            throw new IllegalStateException("cannot peek empty stack");
        }
        return new String(elements[size - 1]);
    }

    public int size() {
        return size;
    }

    public boolean isEmpty() {
        return size == 0;
    }

    public void clear() {
        for (int i = 0; i < size; i++) {
            elements[i] = null;
        }
        size = 0;
    }

    private void ensureCapacity(int minCapacity) {
        if (minCapacity <= elements.length) {
            return;
        }

        int newCapacity = elements.length;
        while (newCapacity < minCapacity) {
            if (newCapacity > Integer.MAX_VALUE / 2) {
                throw new IllegalStateException("capacity overflow");
            }
            newCapacity *= 2;
        }

        String[] expanded = new String[newCapacity];
        System.arraycopy(elements, 0, expanded, 0, size);
        elements = expanded;
    }

    public static void main(String[] args) {
        try {
            SecureStringStack stack = new SecureStringStack(2);
            stack.push("alpha");
            stack.push("beta");
            stack.push("gamma");

            if (!"gamma".equals(stack.peek())) {
                throw new IllegalStateException("peek mismatch after initial pushes");
            }
            System.out.println("peek: " + stack.peek());

            if (stack.size() != 3) {
                throw new IllegalStateException("size mismatch after initial pushes");
            }
            System.out.println("size: " + stack.size());

            if (!"gamma".equals(stack.pop())) {
                throw new IllegalStateException("first pop mismatch");
            }
            System.out.println("pop:  gamma");

            if (!"beta".equals(stack.peek())) {
                throw new IllegalStateException("peek mismatch after pop");
            }
            System.out.println("peek: beta");

            stack.push("delta");
            if (stack.size() != 3) {
                throw new IllegalStateException("size mismatch after delta push");
            }
            System.out.println("size: " + stack.size());

            if (!"delta".equals(stack.pop())) {
                throw new IllegalStateException("delta pop mismatch");
            }
            if (!"beta".equals(stack.pop())) {
                throw new IllegalStateException("beta pop mismatch");
            }
            if (!"alpha".equals(stack.pop())) {
                throw new IllegalStateException("alpha pop mismatch");
            }
            System.out.println("pop sequence: delta, beta, alpha");

            if (!stack.isEmpty()) {
                throw new IllegalStateException("stack should be empty at end of test");
            }
            System.out.println("Java self-test passed");
        } catch (RuntimeException ex) {
            System.err.println("error: " + ex.getMessage());
            System.exit(1);
        }
    }
}
