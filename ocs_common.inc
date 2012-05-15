<?php

define( 'OCS_PARAM_TYPE_STRING', '1');
define( 'OCS_PARAM_TYPE_INTEGER', '2');
define( 'OCS_PARAM_TYPE_ENUM', '3');
define( 'OCS_PARAM_TYPE_ACCOUNT_KEY', '4');
define( 'OCS_PARAM_TYPE_DATE', '5');
define( 'OCS_PARAM_TYPE_TIME', '6');
define( 'OCS_PARAM_TYPE_PRODUCT_KEY', '7');
define( 'OCS_PARAM_TYPE_COUNTER', '8');
define( 'OCS_PARAM_TYPE_MESSAGE', '9');

function ocs_admin_get_param_type( $name)
{
    $optype = array(
        'string' => OCS_PARAM_TYPE_STRING,
        'integer' => OCS_PARAM_TYPE_INTEGER,
        'enum' => OCS_PARAM_TYPE_ENUM,
        'account_key' => OCS_PARAM_TYPE_ACCOUNT_KEY,
        'date' => OCS_PARAM_TYPE_DATE,
        'time' => OCS_PARAM_TYPE_TIME,
        'product_key' => OCS_PARAM_TYPE_PRODUCT_KEY,
        'counter' => OCS_PARAM_TYPE_COUNTER,
        'message' => OCS_PARAM_TYPE_MESSAGE,
    );

    if (isset( $optype[$name])) return $optype[$name];
        
    drupal_set_message( t( 'Undefined Open Parameter Type "@name". It shall be registered as STRING type.', 
        array( '@name' => $name)), 'warning');

    return OCS_PARAM_TYPE_STRING;
}

/*
** @param
    parent_field_name : field name that points parent node
    myid_name : field name that identifies my node
*/
function _ocs_admin_build_path( $list, &$node, $meta_field)
{
    $parent = $node[ $meta_field['plid']];
    $path = array( $node[ $meta_field['mlid']]);
    
    if ( $parent == '0') {
        return $path;
    }
    else {
        if ( isset($list[$parent])) {
            return array_merge( _ocs_admin_build_path( $list, $list[$parent], $meta_field), 
                    $path);
        }
        else {
            // parent not found
            // reset parent group id
            $node[ $meta_field['mlid']] = 0;
            return $path;
        }
    }
}

function _ocs_admin_build_tree( &$tree, $path, $item)
{
    if ( count( $path) == 1) {
        $tree[$item['mlid']] = $item;   // root node
        return;
    }

    // remove the first node
    $elm = $path[0];
    unset( $path[0]);

    return _ocs_admin_build_tree( $tree[$elm]['below'], array_values( $path), $item);
}


/*
    build tree form
    @param meta_field     meta data for building tree
*/
function ocs_admin_build_tree( $nodes, $meta_field, $depth_offset = 0)
{
  // first calculate the depth
  $depthlist = array();
  foreach( $nodes as $node) {
      $path = _ocs_admin_build_path( $nodes, $node, $meta_field);

      $depth = count( $path);
      $depthlist[ $depth][] = array(
        'db' => $node,
        'title' => $node[ $meta_field['title']],
        'mlid' => $node[ $meta_field['mlid']],
        'plid' => $node[ $meta_field['plid']],
        'depth' => $depth + $depth_offset,
        'path' => $path,
        'below' => array(),
      );
  }
//  dpm( $depthlist, 'depthlist');

  // build tree
  $depth = 1;
  $tree = array();
  while( isset( $depthlist[$depth])) {
    // check each depth as one by one
    foreach( $depthlist[ $depth] as $item) {
        // traverse path
  //      dpm( $item['item']['path'], 'path');
        _ocs_admin_build_tree( $tree, $item['path'], $item);
    }

    $depth = $depth +1;
  }

    return $tree;
}

function ocs_admin_tree_table_theme( $form, $header)
{

  drupal_add_tabledrag('menu-overview', 'match', 'parent', 'menu-plid', 'menu-plid', 'menu-mlid', TRUE, MENU_MAX_DEPTH - 1);
  drupal_add_tabledrag('menu-overview', 'order', 'sibling', 'menu-weight');

  $rows = array();
  foreach (element_children($form) as $mlid) {
//    dpm( $mlid, 'mlid');
    if (isset( $form[$mlid]['mlid'])) {
      $element = &$form[$mlid];
      $operations = array();
      foreach( element_children( $element['operations']) as $op) {
          $operations[] = array( 'data' => drupal_render($element['operations'][$op]),
            'class' => array('menu-operations'));
      }

      // Add special classes to be used for tabledrag.js.
      $element['plid']['#attributes']['class'] = array('menu-plid');
      $element['mlid']['#attributes']['class'] = array('menu-mlid');
      $element['weight']['#attributes']['class'] = array('menu-weight');

      // Change the parent field to a hidden. This allows any value but hides the field.
      $element['plid']['#type'] = 'hidden';

      $row = array();
      $row[] = theme('indentation', array('size' => $element['#item']['depth'] - 1)) . drupal_render($element['title']);
      $row[] = drupal_render($element['weight']) . drupal_render($element['plid']) . drupal_render($element['mlid']);
      $row = array_merge( $row, $operations);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'][] = 'draggable';
      $rows[] = $row;
    }
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => $form['#empty_text'], 'colspan' => '7'));
  }

  $footer = ( isset($form['#footer'])) ?  array(array('data' => $form['#footer'], 'colspan' => '7')) : NULL;

  $output = '';
  if ( isset( $form['#caption'])) {
      $output .= '<h3>' . $form['#caption'] . '</h3>';
  } 
  if ( isset( $form['#descriptoin'])) {
	  $output .= $form['#description'];
  }
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 
    'footer' => $footer, 'attributes' => array('id' => 'menu-overview')));
  $output .= drupal_render_children($form);
  return $output;
}

function ocs_admin_table_theme( $form, $header)
{

  drupal_add_tabledrag('menu-overview', 'order', 'sibling', 'menu-weight');

  $rows = array();
//  dpm( $form, 'table form');
  foreach ($form['#records'] as $key => $item) {
      $element = &$form['#records'][$key];
      $operations = array();
      foreach( element_children( $element['operations']) as $op) {
          $operations[] = array( 'data' => drupal_render($element['operations'][$op]),
            'class' => array('menu-operations'));
      }

      // Add special classes to be used for tabledrag.js.
      $element['weight']['#attributes']['class'] = array('menu-weight');

      $row = array();
      $row[] = theme( 'ocs_admin_group_overview', 
            array( 'title' => $element['title'],
                'name' => $element['name'],
                'description' => $element['description']));

      $row[] = drupal_render($element['weight']);
      $row = array_merge( $row, $operations);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'][] = 'draggable';
      $rows[] = $row;
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => $form['#empty_text'], 'colspan' => '7'));
  }
  $footer = ( isset($form['#footer'])) ?  array(array('data' => $form['#footer'], 'colspan' => '7')) : NULL;

  $output = '<br>';
  if ( isset( $form['#caption'])) {
      $output .= '<h3>' . $form['#caption'] . '</h3>';
  }
  $output .= theme('table', array('header' => $header, 'footer' => $footer, 'rows' => $rows, 'attributes' => array('id' => 'menu-overview')));
  return $output;
}

// get new content node
function ocs_admin_get_new_node( $type)
{
    $node = (object) NULL;
    $node->type = $type;
    $node->uid = 0;
    $node->created = strtotime("now");
    $node->changed = strtotime("now");
    $node->status = 1;
    $node->comment = 0;
    $node->promote = 0;
    $node->moderate = 0;
    $node->sticky = 0;
    $node->language = 'und';

    node_object_prepare($node);

    return $node;
}


function ocs_admin_get_counter_name_list()
{
	$counters = &drupal_static( __FUNCTION__, array());
	if ( empty( $counters)) {
        // get counter name list
        module_load_include( 'inc', 'ocs_admin', 'ocs_counter');
        $counter_list = ocs_admin_counter_name_load_all();
        foreach( $counter_list as $value) {
            $counters[ $value['counter_name']] = $value['counter_name'];
        }
    }

    return $counters;
}


function ocs_admin_get_message_list()
{
    $messages = &drupal_static( __FUNCTION__, array());
    if ( empty( $messages)) {   
        $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_messages'));
        foreach( $items as $item) {
            dpm( $item, 'message');
            $messages[$item->nid] = $item->nid . ' : ' . $item->field_format['und'][0]['value'];
        }
    }

    return $messages;
}

function ocs_admin_get_counter_id_list()
{
    $counter_id = &drupal_static( __FUNCTION__, array());
    if ( empty( $counter_id)) {
        module_load_include( 'inc', 'ocs_admin', 'ocs_counter');
        $counter_list = ocs_admin_counter_name_load_all();
        foreach( $counter_list as $counter) {
            $counter_id[ $counter['counter_name']] = $counter['counter_id'];
        }
    }

    return $counter_id;
}

function ocs_admin_ftp_put( $ip_address, $user, $passwd, $dir, $file)
{
    $error_msg = NULL;

    // put file to the OCS server by ftp

    $ftp = ftp_connect( $ip_address);
    if ( $ftp) {
        $result = ftp_login( $ftp, $user, $passwd);
        if ( $result ) {
            if ( ftp_chdir( $ftp, $dir)) {
                if ( ! ftp_put( $ftp, $file, '/tmp/'.$file, FTP_ASCII)) {
                    $error_msg = t('Fail to put file %file to the server %addr at %dir.',
                        array( '%file' => $file, '%addr' => $ip_address, '%dir' => $dir));
                }
            }
            else {
                $error_msg = t('Fail to change directory to %dir', array( '%dir' => $dir));
            }
            ftp_close( $ftp);
        }
        else {
            $error_msg = t('Fail to login to the server %addr with user id "%user".',
                array( '%addr' => $ip_address, '%user' => $user));
        }
    }
    else {
        $error_msg = t('Fail to connect the server %addr',
            array( '%addr' => $ip_address));
    }

    if ( $error_msg) {
        drupal_set_message( $error_msg, 'warning');
        return FALSE;
    }


    return TRUE;
}


