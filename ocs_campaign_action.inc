<?php


function ocs_admin_campaign_action_form( $form, &$form_state, $campaign_id, $rule_id, $action_id = NULL, $op = NULL)
{
	$selected_action = isset( $form_state['values']['ocs_action_id']) ? $form_state['values']['ocs_action_id'] : NULL;

    if ( $op == 'edit') {
        $edit_action = ocs_admin_campaign_action_load( $action_id);
        if ( ! isset( $selected_action)) $selected_action = $edit_action['ocs_action_id'];
        dpm( $edit_action, 'edit');
    }

    $form['#op'] = $op;
    $form['#campaign_id'] = $campaign_id;
    $form['#action_id'] = $action_id;
    $form['#rule_id'] = $rule_id;

    $form['body'] = array(
        '#type' => 'markup',
        '#markup' => t( 'Select the action to add.'),
    );

	$form['event'] = array(
		'#type' => 'item',
		'#title' => t( 'Event : <em>@name</em>', array('@name' => $_GET['event_name'])), 
	);

	$form['rule'] = array(
		'#type' => 'item',
		'#title' => t( 'Rule : <em>@name</em>', array('@name' => $_GET['rule_name'])), 
	);

    $items = entity_load('node', FALSE, array('type' => 'ocs_actions'));
    $options = array();
    foreach( $items as $item) {
//        dpm( $item, 'action');
        $client = taxonomy_term_load( $item->field_ocs_client_id['und'][0]['tid']);
        $options[$client->name][$item->nid] = $item->field_description['und'][0]['safe_value'];
		if ( ! isset($selected_action)) {
            $selected_action = $item->nid;
            $form['#action_name'] = $item->title;
        }
        else if ( $selected_action == $item->nid) {
            $form['#action_name'] = $item->title;
        }
    }

    dpm( $items, 'actions');
    dpm( $selected_action, 'selected');

	$form['ocs_action_id'] = array(
		'#type' => 'select',
		'#title' => t('Action to add'),
		'#options' => $options,
		'#default_value' => $selected_action,
		'#description' => t('The Action shall be applied to the system.'),
		'#ajax' => array(
			'callback' => 'ocs_admin_ajax_action_param',
			'wrapper' => 'campaign_action_param_div',
			),
	);

	// get list of parameters
	$form['param'] = array(
		'#type' => 'fieldset',
		'#title' => 'Parameters',
		'#prefix' => '<div id="campaign_action_param_div">',
		'#suffix' => '</div>',
		);

    $params = new OcsParam;
    $params->load( $selected_action, $rule_id);

    foreach( $params->keylist() as $key) {
        $form['param'][$key] = $params->get_form( $key);
        if ( isset($edit_action) && isset( $edit_action['param'][$key]))  {
            $form['param'][$key]['#default_value'] = $edit_action['param'][$key]['value'];
        }
    }

	$form['#param_name_list'] = $params->keylist();

    $form['actions'] = array('#type' => 'actions');
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


function ocs_admin_ajax_action_param( $form, &$form_state)
{
	return $form['param'];
}

function ocs_admin_campaign_action_form_validate( $form, &$form_state)
{
    $values = $form_state['values'];
    if ( $values['op'] != 'Cancel') {
        $param = array();
        $output = $form['#action_name'];
        foreach( $form['#param_name_list'] as $key) {
            $elm = $form['param'][$key];
            if ( $elm['#type'] == 'textfield') {
                if ( empty( $values[$key])) {
                    form_set_error( $key, 'Invalid textfield value.');
                }
            }
        }
    }
}

function ocs_admin_campaign_action_form_submit( $form, &$form_state)
{
//    dpm( $form_state['values'], 'form_state');
    $values = $form_state['values'];
    if ( $values['op'] != 'Cancel') {
        list( $param, $output) = ocs_admin_get_action_param_list( $form, $values);

        dpm( $form['#action_id'], 'action');

        $action = array(
            'action_id' => $form['#action_id'],
            'rule_id' => $form['#rule_id'],
            'ocs_action_id' => $values['ocs_action_id'],
            'action_name' => $output,
        );

        if ( $form['#op'] == 'edit')
            ocs_admin_campaign_action_save( $action, $param);
        else
            ocs_admin_campaign_action_add( $action, $param);
    }


    $form_state['redirect'] = 'ocs_admin/campaign/' . $form['#campaign_id'] .'/rule';
}

function ocs_admin_get_action_param_list( $form, $values)
{
    $param = array();

    // get action name
    $output = $form['#action_name'];

    // for each parameter name, build param[] array
    foreach( $form['#param_name_list'] as $key) {
        $param[] = array(
            'param_id' => $key,
            'param_name' => $form['param'][$key]['#title'],
            'value' => $values[$key],
            'action_id' => $form['#action_id'],
        );
        $v = $values[$key];
        $output .= t( ' @name=@value', array( '@name' => $form['param'][$key]['#title'],
            '@value' => isset($form['param'][$key]['#options']) ? $form['param'][$key]['#options'][$v] : $v));
    }

    return array( $param, $output);
}

/*
    add new action to the campaign rule
*/
function ocs_admin_campaign_action_add( $action, $params)
{
    db_set_active( 'ocsdb');

    $options = array( 'return' => Database::RETURN_INSERT_ID);
    $action_id = db_insert( 'ocs_campaign_actions', $options) 
        ->fields( $action)
        ->execute();

    if ( $action_id > 0) {
        foreach( $params as $param) {
            $param['action_id'] = $action_id;
            $result = db_insert( 'ocs_campaign_action_param')
                ->fields( $param)
                ->execute();
        }
    }

    db_set_active( 'default');

    return $action_id;
}
 
/*
    save OCS campaign event
*/
function ocs_admin_campaign_action_save( $action, $params)
{
    db_set_active( 'ocsdb');

    $txn = db_transaction();

    try {
        $result = db_merge( 'ocs_campaign_actions')
            ->key( array( 'action_id' => $action['action_id']))
            ->fields( $action)
            ->execute();

        $result = db_delete( 'ocs_campaign_action_param')
            ->condition( 'action_id', $action['action_id'])
            ->execute();

        foreach( $params as $param) {
            $result = db_insert( 'ocs_campaign_action_param')
                ->fields( $param)
                ->execute();
        }
    }
    catch( Exception $e) {
        $txn->rollback();
        watchdog_exception('type', $e);
    }

    db_set_active( 'default');

    return $action['action_id'];
}
 
/*
** delete an action
*/
function ocs_admin_campaign_action_delete_confirm( $form, &$form_state, $campaign_id, $rule_id, $action_id)
{
	$action_name = isset( $_GET['name']) ? $_GET['name'] : $action_id;

    $form['#campaign_id'] = $campaign_id;
    $form['#rule_id'] = $rule_id;
    $form['#action_id'] = $action_id;
    $form['#action_name'] = $action_name;
    $name = ($action_id == 0) ? t( 'All the action') : t('The action "@a"', array( '@a' => $action_name));
    $caption = '<p>' . 
            t('<strong>Warning:</strong> @name of rule "@rule" will be deleted from the Campaign.',
                array( '@name' => $name, '@rule' => $_GET['rule_name'])) .
            '</p>';

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the action "%key"?',
        array( '%key' => $action_name)),
        'ocs_admin/campaign/' . $campaign_id . '/rule',
        $caption, t('Delete'));
}

function ocs_admin_campaign_action_delete_confirm_submit( $form, &$form_state)
{
    $campaign_id = $form['#campaign_id'];
    $action_id = $form['#action_id'];
    $rule_id = $form['#rule_id'];
    $action_name = $form['#action_name'];
    ocs_admin_campaign_delete_action( $rule_id, $action_id);

    drupal_set_message( t('The event %key has been removed from the Campaign.', 
			array( '%key' => $action_name)));

    $form_state['redirect'] = 'ocs_admin/campaign/' . $campaign_id . '/rule';
}

function ocs_admin_campaign_delete_action( $rule_id, $action_id)
{
    db_set_active( 'ocsdb');

    $txn = db_transaction();

    try{
        // delete actions from campaign
        if ( $action_id == 0) {
            $actions = db_select( 'ocs_campaign_actions', 'c')
                ->condition( 'c.rule_id', $rule_id, '=')
                ->fields( 'c', array( 'action_id'))
                ->execute();
            
            while( $action = $actions->fetchAssoc()) {
                // delete action parameters
                $result = db_delete( 'ocs_campaign_action_param')
                    ->condition( 'action_id', $action['action_id'], '=')
                    ->execute();
            }

            $result = db_delete( 'ocs_campaign_actions')
                ->condition( 'rule_id', $rule_id, '=')
                ->execute();
        }
        else {
            // delete action parameters
            $result = db_delete( 'ocs_campaign_action_param')
                ->condition( 'action_id', $action_id, '=')
                ->execute();

            $result = db_delete( 'ocs_campaign_actions')
                ->condition( 'action_id', $action_id, '=')
                ->execute();
        }
    }
    catch( Exception $e) {
        $txn->rollback();
        watchdog_exception('type', $e);
    }

	dpm( $result, 'ocs_campaign_event result');

    db_set_active( 'default');
}


/*
   get action list
   */
function ocs_admin_campaign_actions_load_all( $campaign_id)
{
    db_set_active('ocsdb');

    // get group information
    $query = db_select( 'ocs_campaign_actions', 'c');
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

function ocs_admin_campaign_action_load( $action_id)
{
    db_set_active('ocsdb');

    $query = db_select( 'ocs_campaign_actions', 'c');
    $result = $query->condition( 'c.action_id', $action_id, '=')
        ->fields( 'c')
        ->execute()
        ->fetchAssoc();

    $param = db_select( 'ocs_campaign_action_param', 'c')
        ->condition( 'c.action_id', $action_id, '=')
        ->fields( 'c')
        ->execute()
        ->fetchAllAssoc( 'param_id', PDO::FETCH_ASSOC);

    $result['param'] = $param;
//    dpm( $result, 'action');

    db_set_active( 'default');

    return $result;
}

function ocs_admin_campaign_action_param_load( $action_id)
{
    db_set_active('ocsdb');

    $result = db_select( 'ocs_campaign_action_param', 'c')
        ->condition( 'c.action_id', $action_id, '=')
        ->fields( 'c')
        ->execute()
        ->fetchAllAssoc( 'param_id', PDO::FETCH_ASSOC);

    db_set_active( 'default');

    return $result;
}
