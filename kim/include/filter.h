typedef struct filter{
	void* data;
	float (*filtering) (struct filter* filter, float new_value);
}FILTER;
