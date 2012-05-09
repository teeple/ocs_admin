<?php

function ocs_admin_campaign_add_cond_form( $form, &$form_state, $campaign_id, $rule_id, $cond_id, $op = NULL)
{
	$cond = isset( $form_state['values']['cond']) ? $form_state['values']['cond'] : NULL;
	$cond_idx = isset( $form_state['values']['cond_idx']) ? $form_state['values']['cond_idx'] : 
        isset( $_GET['idx']) ? $_GET['idx'] : time();

	if ( $op == 'edit') {
		// it's editing mode
		$condition = ocs_admin_campaign_cond_load( $cond_id, $cond_idx);
		if ( ! isset( $cond))
            $cond = $condition['rule_id'].':'.$condition['param_id'];
	}


	$form = array();

	$form['#campaign_id'] = $campaign_id;
	$form['#rule_id'] = $rule_id;
	$form['#cond_id'] = $cond_id;
	$form['#op'] = $op;

	$form['event'] = array(
		'#type' => 'item',
		'#title' => t( 'Event : <em>@name</em>', array('@name' => $_GET['event_name'])), 
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
			if ( ! isset( $cond)) {
				$cond = $rule_id . ':' . $param['target_id'];
			}

			// add additional condition if the parameter is 'account key' type
			if ( $para->field_ocs_param_type['und'][0]['value'] == OCS_PARAM_TYPE_ACCOUNT_KEY) {
				$paramlist[$key.':account'] = $options[ $node->title][ $key.':account'] = t( 'account of the @val party', array( '@val' => $value));
				$paramlist[$para->nid.':account'] = OCS_PARAM_TYPE_ACCOUNT_KEY;
				// reset the type of original parameter as string
				$paramlist[$para->nid] = OCS_PARAM_TYPE_STRING;		
			}
		}

		// get counter name list
		module_load_include( 'inc', 'ocs_admin', 'ocs_counter');
		$counter_list = ocs_admin_counter_name_load_all();
		$counters = array();
		foreach( $counter_list as $id => $value) {
			$counters[$id] = $value['counter_name'];
		}

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
    );

    $form['container']['param'] = array(
        '#type' => 'fieldset',
//            '#title' => t('Select Parameter'),
        '#prefix' => '<div id="change_param_div">',
        '#suffix' => '</div>',
        '#attributes' => array('class' => array('column', 'borderless'), 'style' => 'width:40%'),
    );

    $form['container']['param']['cond'] = array(
        '#type' => 'select',
        '#title' => t('Parameter'),
        '#options' => $options,
        '#default_value' => $cond,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_change_param',
            'wrapper' => 'change_param_div',
            ),
    );

    if ( substr( $cond, -7) == 'account') {
        $form['container']['param']['counter'] = array(
            '#type' => 'select',
            '#title' => t('Counter'),
            '#options' => $counters,
        );
    }

    $form['container']['param']['operator'] = array(
        '#type' => 'select',
        '#title' => 'Operator',
        '#options' => _campaign_operator( $paramlist, $cond),
        '#default_value' => isset($condition) ? $condition['operator'] : NULL,
    );

    $form['container']['valueset'] = array(
        '#type' => 'fieldset',
//            '#title' => 'Set Value',
        '#prefix' => '<div id="replace_value_div">',
        '#suffix' => '</div>',
        '#attributes' => array('class' => array( 'borderless'), 'style' => 'width:40%'),
    );

    $direct_input =  isset( $form_state['values']['value_type']) ? $form_state['values']['value_type'] : 1;
    $form['container']['valueset']['value_type'] = array(
        '#type' => 'checkbox',
        '#title' => 'Input direct value',
        '#default_value' => $direct_input,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_value_type',
            'wrapper' => 'replace_value_div',
        ),
    );


    if ( $direct_input ) {
        $form['container']['valueset']['value'] = array(
            '#type' => 'textfield',
            '#title' => 'Value',
            '#size' => 30,
            '#default_value' => isset($condition) ? $condition['value'] : NULL,
        );
    }
    else {
        $form['container']['valueset']['value_list'] = array(
            '#type' => 'select',
            '#title' => 'Customer\'s Account Balance',
            '#options' => $counters,
        );
    }

	$form['actions'] = array( 
        '#type' => 'actions',
        '#weight' => '20',
    );

	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save configuration'),
	);

	$form['actions']['cancel'] = array(
		'#type' => 'submit',
		'#value' => t('Cancel'),
	);


	return $form;
}

function _campaign_operator( $paramlist, $selected)
{
	dpm( $paramlist, 'param');
	dpm( $selected, 'selected');
	$oplist = array(
		OCS_PARAM_TYPE_STRING => array(
			'string', 'contains', 'equal to', 'not equal to'
			),
		OCS_PARAM_TYPE_INTEGER => array(
			'integer', 'equal to', 'not equal to', 'greater than', 'less than'
			),
		OCS_PARAM_TYPE_ENUM => array(
			'enum', 'contains', 'equal to', 'not equal to'
			),
		OCS_PARAM_TYPE_ACCOUNT_KEY => array(
			'account key', 'contains', 'equal to', 'not equal to'
			),
		OCS_PARAM_TYPE_DATE => array(
            ),
		OCS_PARAM_TYPE_TIME => array(
            ),
		OCS_PARAM_TYPE_PRODUCT_KEY => array(
            ),
	);

	$list = explode( ':', $selected);       // event:param:account
    if ( count( $list) == 3) {
        if ( $list[2] == 'account') return drupal_map_assoc( $oplist[OCS_PARAM_TYPE_ACCOUNT_KEY]);
    }
    else if ( isset( $paramlist[ $list[1]])) {  // param
		return drupal_map_assoc( $oplist[$paramlist[$list[1]]]);
	}

	return drupal_map_assoc( array('unknown parameter type'));
}

function ocs_admin_ajax_change_param( $form, $form_state)
{
	return $form['container']['param'];
}

function ocs_admin_ajax_value_type( $form, $form_state)
{
	return $form['container']['valueset'];
}



function ocs_admin_campaign_add_cond_form_submit( $form, &$form_state)
{
	$values = $form_state['values'];
//	dpm( $values, 'values submit');
	if ( $values['op'] != 'Cancel') {
		$list = explode( ':', $values['cond']);     // format : 'rule_id:param_id:string'
		$cond = array(
            'cond_id' => $form['#cond_id'],
			'rule_id' => $list[0],
			'cond_idx' => $values['cond_idx'],
			'param_id' => $list[1] . (isset($list[2]) ? ':'.$list[2] : ''),
			'param_name' => $form['#paramlist'][ $values['cond']],
			'operator' => $values['operator'],
			'value' => $values['value'],
			'cond_name' => t( '@para @op @value', array( 
					'@para' =>  $form['#paramlist'][ $values['cond']],
					'@op' => $values['operator'],
					'@value' => $values['value'])),
		);
//        dpm( $cond, 'condition to submit');
        if ( $form['#op'] == 'edit')
            ocs_admin_campaign_condition_save( $cond);
        else
            ocs_admin_campaign_condition_add( $cond);
	}

	$form_state['redirect'] = 'ocs_admin/campaign/' . $form['#campaign_id'] . '/rule';
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

	$result = db_insert( 'ocs_campaign_conditions')
		->fields( $cond)
		->execute();
    dpm( $result, 'db_insert result');

    db_set_active( 'default');

	return $result;
}


function ocs_admin_campaign_cond_delete_confirm( $form, &$form_state, $campaign_id, $cond_id)
{
	$cond_name = isset( $_GET['name']) ? $_GET['name'] : 'Condition ID:' . $cond_id;

    $form['#campaign_id'] = $campaign_id;
    $form['#cond_id'] = $cond_id;
    $form['#cond_name'] = $cond_name;
    $form['#cond_idx'] = isset( $_GET['idx']) ? $_GET['idx'] : '0';

    $caption = '<p>' . 
            t('<strong>Warning:</strong> The condition "%key" will be deleted from the Campaign.',
                array( '%key' => $cond_name)) .
            '</p>';

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the event "%key"?',
        array( '%key' => $cond_name)),
        'ocs_admin/campaign/' . $campaign_id . '/rule',
        $caption, t('Delete'));
}

function ocs_admin_campaign_cond_delete_confirm_submit( $form, &$form_state)
{
    $campaign_id = $form['#campaign_id'];
    $cond = array(
        'cond_id' => $form['#cond_id'],
        'cond_idx' => $form['#cond_idx'],
    );
    ocs_admin_campaign_condition_delete( $cond);

    drupal_set_message( t('The condition "%key" has been removed from the Campaign.', 
			array( '%key' => $form['#cond_name'])));

    $form_state['redirect'] = 'ocs_admin/campaign/' . $campaign_id . '/rule';
}

function ocs_admin_campaign_condition_delete( $cond)
{
    db_set_active( 'ocsdb');

	// delete event from campaign
	$result = db_delete( 'ocs_campaign_conditions')
		->condition( 'cond_id', $cond['cond_id'], '=')
		->condition( 'cond_idx', $cond['cond_idx'], '=')
		->execute();
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

    dpm( $records, 'conditions');

    db_set_active( 'default');

    return $records;
}

function ocs_admin_campaign_rule_save( $rule)
{
    db_set_active('ocsdb');

	$result = db_insert( 'ocs_campaign_rules')
		->fields( $rule)
		->execute();
    dpm( $result, 'db_merge result');

    db_set_active( 'default');

	return $result;
}


function ocs_admin_campaign_rule_delete( $rule_id)
{
    db_set_active('ocsdb');

	$result = db_delete( 'ocs_campaign_rules')
		->condition( 'rule_id', $rule_id, '=')
		->execute();

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


