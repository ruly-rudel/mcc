struct list {
	struct list *next;
	int val;
};

int *calloc();

struct list *
push_front(struct list *lst, int val)
{
	struct list* head = calloc(1, sizeof (*lst));
	head->val = val;
	head->next = lst;

	return head;
}

struct list *
push_back(struct list *lst, int val)
{
	struct list* head = lst;
	struct list* tail = calloc(1, sizeof (*lst));
	if(lst)
	{
		for(; lst->next; lst = lst->next);
		lst->next = tail;

	}
	tail->val = val;
	tail->next = 0;

	return head;
}

int main()
{
	struct list *lst = 0;
	int sum = 0;
	lst = push_front(lst, 1);
	lst = push_front(lst, 2);
	lst = push_front(lst, 3);
	lst = push_front(lst, 4);
	lst = push_back(lst, 5);
	lst = push_back(lst, 6);
	for(; lst; lst = lst->next)
	{
		sum = sum + lst->val;
	}

	return sum;
}
