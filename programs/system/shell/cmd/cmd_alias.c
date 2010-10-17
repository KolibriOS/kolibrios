
int cmd_alias(char arg[])
{

int result;

if (NULL == arg)
	{
	alias_list();
	return TRUE;
	}

result = alias_check(arg);

if ( ( 0 != result ) && ( -1 != result ) )
	alias_add(arg);

return TRUE;
}
