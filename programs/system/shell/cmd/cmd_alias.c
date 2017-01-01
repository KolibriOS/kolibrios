
int cmd_alias(char arg[])
{

int result;

if (NULL == arg || '\0' == arg[0])
	{
	alias_list();
	return TRUE;
	}

result = alias_check(arg);

if ( ( 0 != result ) && ( -1 != result ) )
	alias_add(arg);

return TRUE;
}

