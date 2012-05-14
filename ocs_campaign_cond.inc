<?php


function ocs_admin_campaign_add_cond_form( $form, &$form_state, $campaign_id, $rule_id, $cond_id, $op = NULL)
{
	$selected_param = isset( $form_state['values']['param']) ? $form_state['values']['param'] : NULL;
	$cond_idx = isset( $_GET['idx']) ? $_GET['idx'] : 
        (isset( $form_state['values']) ? $form_state['values']['cond_idx'] : time());
    $operator = isset( $form_state['values']) ? $form_state['values']['operator'] : NULL;
    $direct_input = isset( $form_state['values']['direct_input']) ? $form_state['values']['direct_input'] : 1;

	if ( $op == 'edit') {
		// it's editing mode
		$condition = ocs_admin_campaign_cond_load( $cond_id, $cond_idx);

		if ( ! isset( $selected_param))
            $selected_param = $condition['rule_id'].':'.$condition['param_id'];
        if ( ! isset( $operator))
            $operator = $condition['operator'];
        if ( ! isset( $form_state['values']['direct_input']))
            $direct_input = $condition['direct_input'];
	}


	$form = array();

	$form['#campaign_id'] = $campaign_id;
	$form['#rule_id'] = $rule_id;
	$form['#cond_id'] = $cond_id;
	$form['#op'] = $op;

	$form['event'] = array(
		'#type' => 'item',
		'#title' => t( 'Event : <em>@name</em>', array('@name' => strip_tags($_GET['event_name']))), 
	);

	$form['rule'] = array(
		'#type' => 'item',
		'#title' => t( 'Rule : <em>@name</em>', array('@name' => strip_tags($_GET['rule_name']))), 
	);

	$rule = &drupal_static( __FUNCTION__ . 'rule', array());
    if ( empty( $rule)) {
        // get event info
        $rule = ocs_admin_campaign_rule_load( $rule_id);
        if ( empty( $rule)) {
            drupal_set_message( t('Unable to find rule %id.', array( '%id' => $rule_id)), 'error');
            $form['actions'] = array('#type' => 'actions');
            $form['actions']['cancel'] = array(
                '#type' => 'submit',
                '#value' => t('Cancel'),
            );

            return $form;
        }
    }
//    dpm( $rule, 'rule');

	$form['#event_id'] = $event_id = $rule['event_id'];

	$options = &drupal_static( __FUNCTION__ . 'options', array());
	$paramlist = &drupal_static( __FUNCTION__ . 'param', array());
	if ( empty( $options) || empty( $param)) {
		$node = node_load( $event_id);
//		dpm( $node, 'node');

        list( $paramlist, $options, $selected) = ocs_admin_get_param_list( $node, $rule_id);
        if ( ! isset( $selected_param)) $selected_param = $selected;
//        dpm( $paramlist, 'paramlist');
	}

//    dpm( $paramlist, 'param list');

	$form['#paramlist'] = $paramlist;

    $form['cond_idx'] = array(
        '#type' => 'hidden',
        '#value' => $cond_idx,
    );

    $form['container'] = array(
        '#type' => 'fieldset',
        '#title' => t( 'Condition'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#description' => t('Set the parameter and value.'),
        '#prefix' => '<div id="change_param_div">',
        '#suffix' => '</div>',
    );

    $form['container']['param'] = array(
        '#type' => 'fieldset',
//            '#title' => t('Select Parameter'),
        '#attributes' => array('class' => array('column', 'borderless'), 'style' => 'width:40%'),
    );

    $form['container']['param']['param'] = array(
        '#type' => 'select',
        '#title' => t('Parameter'),
        '#options' => $options,
        '#default_value' => $selected_param,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_change_param',
            'wrapper' => 'change_param_div',
            ),
    );

    if ( substr( $selected_param, -7) == 'account') {
        $counters = ocs_admin_get_counter_name_list();
        $form['container']['param']['counter'] = array(
            '#type' => 'select',
            '#title' => t('Counter'),
            '#options' => $counters,
        );
    }

    $form['container']['param']['operator'] = array(
        '#type' => 'select',
        '#title' => 'Operator',
        '#options' => _campaign_operator( $paramlist, $selected_param),
        '#default_value' => isset($condition) ? $condition['operator'] : NULL,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_value_type',
            'wrapper' => 'replace_value_div',
            ),
    );

    $form['container']['valueset'] = array(
        '#type' => 'fieldset',
//            '#title' => 'Set Value',
        '#prefix' => '<div id="replace_value_div">',
        '#suffix' => '</div>',
        '#attributes' => array('class' => array( 'borderless'), 'style' => 'width:40%'),
    );

    $form['container']['valueset']['direct_input'] = array(
        '#type' => 'checkbox',
        '#title' => 'Input direct value',
//        '#default_value' => $direct_input,
        '#default_value' => $direct_input,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_value_type',
            'wrapper' => 'replace_value_div',
        ),
    );


    $form['container']['valueset']['value2'] = array(
        '#type' => 'hidden',
    );

//    dpm( $direct_input, 'dir input');
    if ( ! empty($direct_input) || $operator == 'between' ) {
        $form['container']['valueset']['value'] = ocs_admin_get_param_form( $paramlist, $selected_param, $operator);
        dpm( $form['container']['valueset']['value'], 'direct input');

        if ( $operator == 'between') {
            $form['container']['valueset']['value2'] = $form['container']['valueset']['value'];
            unset( $form['container']['valueset']['value2']['#title']);
        }
        if ( isset($condition))  {
            if ( $form['container']['valueset']['value']['#type'] == 'textfield') {
                $form['container']['valueset']['value']['#default_value'] = $condition['value'];
                $form['container']['valueset']['value2']['#default_value'] = $condition['value2'];
            }
            else {
                $json = json_decode( $condition['value'], TRUE);
                dpm( $json, 'json');
                if ( $operator == 'in list') {
                    $form['container']['valueset']['value']['#default_value'] = $json;
                }
                else {
                    $v = array_values( $json);
                    $form['container']['valueset']['value']['#default_value'] = $v[0];
                }
            }
        }
    }
    else {
        $counters = ocs_admin_get_counter_name_list();
        $form['container']['valueset']['value'] = array(
            '#type' => 'select',
            '#title' => 'Customer\'s Account Balance',
            '#options' => $counters,
        );
        if ( isset($condition))  {
            $form['container']['valueset']['value']['#default_value'] = $condition['value'];
        }
    }

	$form['actions'] = array( 
        '#type' => 'actions',
        '#weight' => '20',
    );

	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Configuration'),
	);

	$form['actions']['cancel'] = array(
		'#type' => 'submit',
		'#value' => t('Cancel'),
	);


	return $form;
}

function ocs_admin_get_param_form( $paramlist, $param, $operator = NULL)
{
    $type = _get_param_type( $paramlist, $param);
    $title = isset( $paramlist[$param.':title']) ? $paramlist[$param.':title'] : 'Value';

    if ($type == OCS_PARAM_TYPE_ENUM) {
        $options = array_flip( $paramlist[$param.':enum']);
        $form = array(
            '#type' => ($operator == 'in list') ? 'checkboxes' : 'radios',
            '#title' => $title,
            '#options' => $options,
        );
    }
    else if ( $type == OCS_PARAM_TYPE_COUNTER) {
        $counters = ocs_admin_get_counter_name_list();
        $form = array(
            '#type' => 'select',
            '#title' => $title,
            '#options' => $counters,
        );
    }
    else {
        $form = array(
            '#type' => 'textfield',
            '#title' => $title,
            '#size' => 30,
        );
    }

    return $form;
}

// build parameter list info dictionary
function ocs_admin_get_param_list( $node, $rule_id)
{
    $paramlist = array();
    $options = array();

    foreach( $node->field_ocs_param_ref['und'] as $param) {
        $para = node_load( $param['target_id']);
//				dpm( $para, 'parameter');

        // store select options
        $key = $rule_id . ':' . $param['target_id'];
        $value = $para->title;
        $options[ $node->title][ $key] = $value;
        // store parameter info
        $paramlist[$para->nid] = $para->field_ocs_param_type['und'][0]['value'];
        $paramlist[$key] = $value;
        $paramlist[$key.':title'] = $para->title;
        if ( ! isset( $selected_param)) {
            $selected_param = $rule_id . ':' . $param['target_id'];
        }

        // add additional condition if the parameter is 'account key' type
        if ( $para->field_ocs_param_type['und'][0]['value'] == OCS_PARAM_TYPE_ACCOUNT_KEY) {
            $paramlist[$key.':account'] = $options[ $node->title][ $key.':account'] = t( 'counter of the @val party', array( '@val' => $value));
            $paramlist[$para->nid.':account'] = OCS_PARAM_TYPE_ACCOUNT_KEY;
            // reset the type of original parameter as string
            $paramlist[$para->nid] = OCS_PARAM_TYPE_STRING;		
        }
        else if ( $para->field_ocs_param_type['und'][0]['value'] == OCS_PARAM_TYPE_ENUM) {
            // add enum value list
            $paramlist[$key.':enum'] = json_decode( $para->field_ocs_param_value['und'][0]['value'], TRUE);
        }
    }

    return array( $paramlist, $options, $selected_param);
}

function _campaign_operator( $paramlist, $selected)
{
//	dpm( $paramlist, 'param');
//	dpm( $selected, 'selected');

    $type = _get_param_type( $paramlist, $selected);
	$oplist = array(
		OCS_PARAM_TYPE_STRING => array(
			'=', 'starts', 
			),
		OCS_PARAM_TYPE_INTEGER => array(
			'=', '>', '>=', '<', '<=', 'between',
			),
		OCS_PARAM_TYPE_ENUM => array(
			'=', 'in list',
			),
		OCS_PARAM_TYPE_ACCOUNT_KEY => array(
			'=', '>', '>=', '<', '<=', 'between',
			),
		OCS_PARAM_TYPE_DATE => array(
			'=', '>', '>=', '<', '<=', 'between',
            ),
		OCS_PARAM_TYPE_TIME => array(
			'=', '>', '>=', '<', '<=', 'between',
            ),
		OCS_PARAM_TYPE_PRODUCT_KEY => array(
			'=', '>', '>=', '<', '<=', 'between',
            ),
	);

	return drupal_map_assoc( $oplist[$type]);
}


function _get_param_type( $paramlist, $selected)
{
	$list = explode( ':', $selected);       // event:param:account or action:param:account

    if ( substr( $selected, -7) == 'account') {
        return OCS_PARAM_TYPE_ACCOUNT_KEY;
    }
    else if ( substr( $selected, -4) == 'enum') {
        return OCS_PARAM_TYPE_ENUM;
    }
    else if ( isset( $paramlist[$list[1]])) {
        return $paramlist[$list[1]];
	}

    return OCS_PARAM_TYPE_STRING;
}

function ocs_admin_ajax_change_param( $form, $form_state)
{
	return $form['container'];
}

function ocs_admin_ajax_value_type( $form, $form_state)
{
	return $form['container']['valueset'];
}


function ocs_admin_campaign_add_cond_form_validate( $form, &$form_state)
{
    $values = $form_state['values'];
//    dpm( $values, 'form state');
    if ( $values['direct_input']) {
        // direct input

        $type = _get_param_type( $form['#paramlist'], $values['param']);
        $msg = ocs_admin_check_param_value( $type, $values['value']);
        if ( ! empty( $msg)) {
            form_set_error( 'value', $msg);
        }

        if ( $form['container']['valueset']['value2']['#type'] != 'hidden') {
            $msg = ocs_admin_check_param_value( $type, $values['value2']);
            if ( ! empty( $msg)) {
                form_set_error( 'value2', $msg);
            }
        }
    }

}

function ocs_admin_check_param_value( $type, $value)
{
    $t_arg = array( '%v' => $value);
    switch( $type) {
        case OCS_PARAM_TYPE_INTEGER:
        case OCS_PARAM_TYPE_ACCOUNT_KEY:    // digits
            if ( ! is_numeric($value)) {
                return t('Invalid value "%v". Must be numeric.', $t_arg);
            }
            break;
        case OCS_PARAM_TYPE_DATE:   // yyyyMMdd, MMdd
            $length = strlen( $value);
            if ( ! is_numeric($value) || !($length == 4 || $length == 8)) {
                return t('Invalid date value "%v". Must be in "yyyyMMdd" format.', $t_arg);
            }
            $year = ($length == 8) ? substr( $value, 0, 4) : date('Y');
            $month = substr( $value, -4, 2);
            $day = substr( $value, -2);
            if ( ! checkdate( $month, $day, $year)) {
                $t_arg['%year'] = $year;
                $t_arg['%mon'] = $month;
                $t_arg['%day'] = $day;
                return t('Invalid date value "%v":%year-%mon-%day. Must be in "yyyyMMdd" format.', $t_arg);
            }

            break;
        case OCS_PARAM_TYPE_TIME:   // HHMM
            if ( ! is_numeric($value) || $value < 0 || $value > 2400) {
                return t('Invalid time value "%v", Must be in "HHMM" format.', $t_arg);
            }
            break;

        case OCS_PARAM_TYPE_STRING:
            $length = strlen( $value);
            if ( $length == 0) return t('Value cannot be empty.');
            break;

        case OCS_PARAM_TYPE_ENUM:
            break;
    }

    return NULL;
}

function ocs_admin_campaign_add_cond_form_submit( $form, &$form_state)
{
	$values = $form_state['values'];
	dpm( $values, 'values submit');
	if ( $values['op'] != 'Cancel') {
		$list = explode( ':', $values['param']);     // format : 'rule_id:param_id:string'
        $value1 = _get_param_value( $form['#paramlist'], $values['param'], $values['value']);
        $value2 = _get_param_value( $form['#paramlist'], $values['param'], $values['value2']);
		$cond = array(
            'cond_id' => $form['#cond_id'],
			'rule_id' => $list[0],
			'cond_idx' => $values['cond_idx'],
			'param_id' => $list[1] . (isset($list[2]) ? ':'.$list[2] : ''),
			'param_name' => $form['#paramlist'][ $values['param']],
			'operator' => $values['operator'],
			'direct_input' => $values['direct_input'],
			'value' => $value1,
			'value2' => $value2,
            'cond_rule' => t('@para !op @value @value2', array(
					'@para' =>  _get_param_name( $form['#paramlist'], $values['param'], $values['counter']),
					'!op' => $values['operator'],
					'@value' => $values['value'],
                    '@value2' => ($values['operator'] == 'between') ? ', '. $values['value2'] : '')),
			'cond_name' => t( '@counter @para @op @value @value2', array( 
                    '@counter' => isset($values['counter']) ? $values['counter'] : '',
					'@para' =>  $form['#paramlist'][ $values['param']],
					'@op' => $values['operator'],
					'@value' => $value1,
                    '@value2' => ($values['operator'] == 'between') ? 'and '. $value2 : '')),
		);
        dpm( $cond, 'condition to submit');
        if ( $form['#op'] == 'edit')
            ocs_admin_campaign_condition_save( $cond);
        else
            ocs_admin_campaign_condition_add( $cond);
	}

	$form_state['redirect'] = 'ocs_admin/campaign/' . $form['#campaign_id'] . '/rule';
}

function _get_param_name( $paramlist, $param, $counter)
{
    $type = _get_param_type( $paramlist, $param);
    if ( $type == OCS_PARAM_TYPE_ACCOUNT_KEY) {
        return $counter;
    }

    return $paramlist[ $param];
}


function _get_param_value( $paramlist, $param, $value)
{
    $type = _get_param_type( $paramlist, $param);
    if ( $type == OCS_PARAM_TYPE_ENUM) {
        $enum_list = array_flip( $paramlist[$param.':enum']);

        dpm( $value, 'param value');

        $selected = array();
        if ( is_array( $value)) {
            foreach( $value as $key => $item) {
                if ( $item != 0) {  // it's selected
                    $selected[$enum_list[$key]] = $key;
                }
            }
        }
        else {
            if ( ! isset( $enum_list[$value])) return NULL;
            $selected = array( $enum_list[$value] => $value);
        }

        // return json encoded string
        return json_encode( $selected);
    }

    return $value;
}

/*
    save OCS campaign condition
*/
function ocs_admin_campaign_condition_save( $cond)
{
    db_set_active('ocsdb');

	$result = db_update( 'ocs_campaign_conditions')
		->fields( $cond)
		->condition( 'cond_id', $cond['cond_id'], '=')
        ->condition( 'cond_idx', $cond['cond_idx'], '=')
		->execute();
    dpm( $result, 'db_update result');

    db_set_active( 'default');

	return $result;
}

/*
    add OCS campaign condition
*/
function ocs_admin_campaign_condition_add( $cond)
{
    db_set_active('ocsdb');
    //dpm( $cond, 'cond');

	$result = db_insert( 'ocs_campaign_conditions')
		->fields( $cond)
		->execute();
    dpm( $result, 'db_insert result');

    db_set_active( 'default');

	return $result;
}


function ocs_admin_campaign_cond_delete_confirm( $form, &$form_state, $campaign_id, $rule_id, $cond_id)
{
	$cond_name = isset( $_GET['name']) ? strip_tags($_GET['name']) : 'Condition ID:' . $cond_id;

    $form['#campaign_id'] = $campaign_id;
    $form['#rule_id'] = $rule_id;
    $form['#cond_id'] = $cond_id;
    $form['#cond_name'] = $cond_name;
    $form['#cond_idx'] = isset( $_GET['idx']) ? $_GET['idx'] : '0';

    $caption = t('<p><strong>Warning:</strong> @name of the Rule @id will be deleted from the Campaign.</p>',
        array( '@id' => $rule_id, '@name' => ($cond_id == 0) ? t('All the conditions') : 
            t( 'The condition "@key"', array( '@key' => $cond_name))));

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the condition ?'),
        'ocs_admin/campaign/' . $campaign_id . '/rule',
        $caption, t('Delete'));
}

function ocs_admin_campaign_cond_delete_confirm_submit( $form, &$form_state)
{
    $campaign_id = $form['#campaign_id'];
    $cond = array(
        'rule_id' => $form['#rule_id'],
        'cond_id' => $form['#cond_id'],
        'cond_idx' => $form['#cond_idx'],
    );
    ocs_admin_campaign_condition_delete( $cond);

    drupal_set_message( t('The condition has been removed from the Campaign.'));

    $form_state['redirect'] = 'ocs_admin/campaign/' . $campaign_id . '/rule';
}

function ocs_admin_campaign_condition_delete( $cond)
{
    db_set_active( 'ocsdb');
    dpm( $cond, 'cond delete');

	// delete event from campaign
    $query = db_delete( 'ocs_campaign_conditions');
    if ( $cond['cond_id'] == 0) {
        $query->condition( 'rule_id', $cond['rule_id'], '=');
    }
    else {
        $query->condition( 'cond_id', $cond['cond_id'], '=')
            ->condition( 'cond_idx', $cond['cond_idx'], '=');
    }

    $result = $query->execute();

	dpm( $result, 'ocs_campaign_condition result');

    db_set_active( 'default');
}


function ocs_admin_campaign_cond_load( $cond_id, $cond_idx)
{
    db_set_active( 'ocsdb');

    $query = db_select( 'ocs_campaign_conditions', 'c');
    $result = $query->condition( 'c.cond_id', $cond_id, '=')
        ->condition( 'c.cond_idx', $cond_idx, '=')
        ->fields( 'c')
        ->execute()
        ->fetchAssoc();

    db_set_active( 'default');

	return $result;
}

function ocs_admin_campaign_rule_load( $rule_id)
{
    db_set_active( 'ocsdb');

    $query = db_select( 'ocs_campaign_rules', 'c');
    $result = $query->condition( 'c.rule_id', $rule_id, '=')
        ->fields( 'c')
        ->execute()
        ->fetchAssoc();
 
    db_set_active( 'default');

	return $result;
}


/*
   add or delete rule

   */
function ocs_admin_ajax_add_campaign_rule( $campaign_id, $event_id, $js)
{
    // add a new rule
    $rule = array(
        'campaign_id' => $campaign_id,
        'event_id' => $event_id,
    );

    ocs_admin_campaign_rule_save( $rule);

	$form = drupal_get_form( 'ocs_admin_campaign_rule_form', $campaign_id);

    // event table
    $output = ocs_admin_campaign_rule_theme( $form);

	if ( $js) {
		ctools_include('ajax');
		$commands = array();
		$commands[] = ajax_command_replace( '#ocs_admin_campaign_rule_div', $output);
		print ajax_render( $commands);
		exit;
	}
	else {
		return $output;
	}
}

function ocs_admin_add_campaign_rule_form( $form, &$form_state, $campaign_id, $event_or_rule_id, $op = NULL)
{
    $form['#campaign_id'] = $campaign_id;

	$form['event'] = array(
		'#type' => 'item',
		'#title' => t( 'Event : <em>@name</em>', array('@name' => strip_tags($_GET['event_name']))), 
	);

    $form['rule_name'] = array(
        '#type' => 'textfield',
        '#title' => 'Rule Name',
        '#size' => 30,
    );

    if ( $op == 'edit') {
        // read rule
        $rule = ocs_admin_campaign_rule_load( $event_or_rule_id);
        dpm( $rule, 'rule');
        $form['rule_name']['#default_value'] = $rule['rule_name'];
        $form['#rule_id'] = $event_or_rule_id;
    }
    else {
        $form['#event_id'] = $event_or_rule_id;
    }

	$form['actions'] = array( 
        '#type' => 'actions',
        '#weight' => '20',
    );

	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save'),
	);

	$form['actions']['cancel'] = array(
		'#type' => 'submit',
		'#value' => t('Cancel'),
	);


    return $form;
}

function ocs_admin_add_campaign_rule_form_validate( $form, &$form_state)
{
    $values = $form_state['values'];
    if ( empty( $values['rule_name'])) {
        form_set_error( 'rule_name', 'Invalid rule name.');
    }
}


function ocs_admin_add_campaign_rule_form_submit( $form, &$form_state)
{
    if ( $form_state['values']['op'] != 'Cancel') {
        // add a new rule
        $rule = array(
            'campaign_id' => $form['#campaign_id'],
            'rule_name' => $form_state['values']['rule_name'],
        );
        if ( isset( $form['#event_id'])) $rule['event_id'] = $form['#event_id'];
        if ( isset( $form['#rule_id'])) $rule['rule_id'] = $form['#rule_id'];

//        dpm( $rule, 'rule');
        ocs_admin_campaign_rule_save( $rule);
    }

	$form_state['redirect'] = 'ocs_admin/campaign/' . $form['#campaign_id'] . '/rule';
}


/*
    get condition list
*/
function ocs_admin_campaign_conditions_load_all( $campaign_id)
{
    db_set_active('ocsdb');

    // get group information
    $query = db_select( 'ocs_campaign_conditions', 'c');
    $query->join( 'ocs_campaign_rules', 'r', 'r.rule_id = c.rule_id');  // join with rules table
    $result = $query->condition( 'r.campaign_id', $campaign_id, '=')
        ->fields( 'c')
        ->execute();

    $records = array();
    while( $record = $result->fetchAssoc()) 
    {
        $records[ $record['rule_id']][] = $record;
    }

    db_set_active( 'default');

    return $records;
}

function ocs_admin_campaign_rule_save( $rule)
{
    db_set_active('ocsdb');

	$result = db_merge( 'ocs_campaign_rules')
        ->key( array( 'rule_id' => isset($rule['rule_id']) ? $rule['rule_id'] : NULL))
		->fields( $rule)
		->execute();
    dpm( $result, 'db_merge result');

    db_set_active( 'default');

	return $result;
}


function ocs_admin_campaign_rule_delete( $rule_id)
{
    db_set_active('ocsdb');

    $txn = db_transaction();

    try {
        // delete rules
        $result = db_delete( 'ocs_campaign_rules')
            ->condition( 'rule_id', $rule_id, '=')
            ->execute();

        // delete conditions
        $result = db_delete( 'ocs_campaign_conditions')
            ->condition( 'rule_id', $rule_id, '=')
            ->execute();

        // delete actions
        $result = db_delete( 'ocs_campaign_actions')
            ->condition( 'rule_id', $rule_id, '=')
            ->execute();
    }
    catch( Exception $e) {
        $txn->rollback();
        watchdog_exception( 'type', $e);
    }

    db_set_active( 'default');

	return $result;
}

function ocs_admin_campaign_rule_delete_confirm( $form, &$form_state, $campaign_id, $rule_id)
{
    $name = 'Rule ID:'.$rule_id;
    $form['#campaign_id'] = $campaign_id;
    $form['#rule_id'] = $rule_id;

    $caption = '<p>' . 
            t('<strong>Warning:</strong> The "@name" will be deleted from the Campaign.',
                array( '@name' => $name)) .
            '</p>';

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the rule "%key"?',
        array( '%key' => $name)),
        'ocs_admin/campaign/' . $campaign_id . '/rule',
        $caption, t('Delete'));
}

function ocs_admin_campaign_rule_delete_confirm_submit( $form, &$form_state)
{
    $campaign_id = $form['#campaign_id'];
    $rule_id = $form['#rule_id'];

    ocs_admin_campaign_rule_delete( $rule_id);

    drupal_set_message( t('The Rule %key has been removed from the Campaign.', 
			array( '%key' => $rule_id)));

    $form_state['redirect'] = 'ocs_admin/campaign/' . $campaign_id . '/rule';
}


