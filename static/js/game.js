var WIDTH = 7
var HEIGHT = 6
var playerToken;
var compToken;
var board = [];
var positions = [];
for (var i = 0; i < HEIGHT; i++) {
  positions.push([])
  board.push([])
  for (var j = 0; j < WIDTH; j++) {
    positions[i].push(j);
    board[i].push("");
  }
}
var isPlayerTurn = false;
var skipCounter = 0;

function runAI(board) {
  $("#turn").text("It is the AI's turn. Please wait!");
  $("#turn").css("color", "red");
  // serialise board into a format that the ai likes
  boardStr = "";
  for (var y = HEIGHT-1; y >= 0; y--) {
    for (var x = 0; x < WIDTH; x++) {
      if (board[y][x] == playerToken) {
        boardStr += 'y';
      } else if (board[y][x] == compToken) {
        boardStr += 'r';
      } else {
        boardStr += '.';
      }
    }
    boardStr += ','
  }
  boardStr.slice(0, -1); // remove last comma
  $.post('/ai', {
    board: boardStr,
    player: 'yellow'
  }, function (data) {
    // place our token
    var col = parseInt(data);
    // this means ai is skipping
    if (col === 1000000) {
      return;
    }
    skipCounter = 0;
    for (var i = HEIGHT-1; i >= 0; i--) {
      if (board[i][col] == '') {
        board[i][col] = compToken;
        $("#h" + i + "w" + col).text(compToken);
        return;
      }
    }
  }).then(() => {
    if (!checkWin()) {
      isPlayerTurn = true;
      playerTurn();
    }
  })
}

function checkWin() {
  if (checkWinTok(playerToken)) {
    setTimeout(function () { winner(playerToken) }, 2000);
    return true;
  } else if (checkWinTok(compToken)) {
    setTimeout(function () { winner(compToken) }, 2000);
    return true;
  } else if (checkDraw()) {
    setTimeout(draw, 1000);
    return true;
  }
  return false;
}

function checkWinTok(token) {
  // horizontal
  for (var i = 0; i < HEIGHT; i++) {
    var count = 0;
    for (var j = 0; j < WIDTH; j++) {
      if (board[i][j] == token) {
        count++;
      } else {
        count = 0;
      }
      if (count == 4) {
        for (var k = j; k > j - 4; k--) {
          $("#h" + i + "w" + k).addClass("blinking");
        }
        return true;
      }
    }
  }
  // vertical
  for (var i = 0; i < WIDTH; i++) {
    count = 0;
    for (var j = 0; j < HEIGHT; j++) {
      if (board[j][i] == token) {
        count++;
      } else {
        count = 0;
      }
      if (count == 4) {
        for (var k = j; k > j - 4; k--) {
          $("#h" + k + "w" + i).addClass("blinking");
        }
        return true;
      }
    }
  }
  // diagonals
  // 'down and right' diagonals
  for (var i = 0; i < HEIGHT-3; i++) {
    for (var j = 0; j < WIDTH-3; j++) {
      count = 0
      for (var k = 0; k < 4; k++) {
        if (board[i+k][j+k] == token) {
          count++;
        }
      }
      if (count == 4) {
        for (var k = 0; k < 4; k++) {
          $("#h" + (i+k) + "w" + (j+k)).addClass("blinking");
        }
        return true;
      }
    }
  }
  // 'down and left' diagonals
  for (var i = 0; i < HEIGHT-3; i++) {
    for (var j = 3; j < WIDTH; j++) {
      count = 0
      for (var k = 0; k < 4; k++) {
        if (board[i+k][j-k] == token) {
          count++;
        }
      }
      if (count == 4) {
        for (var k = 0; k < 4; k++) {
          $("#h" + (i+k) + "w" + (j-k)).addClass("blinking");
        }
        return true;
      }
    }
  }
  return false;
}

function checkDraw() {
  // if players are just skipping alot quit.
  if (skipCounter >= 3) {
    return true
  }
  var count = 0;
  for (var i = 0; i < HEIGHT; i++) {
    for (var j = 0; j < WIDTH; j++) {
      if (board[i][j] == playerToken || board[i][j] == compToken) {
        count++;
      }
    }
  }
  if (count == HEIGHT * WIDTH) {
    return true;
  }
  return false;
}

function winner(token) {
  for (var i = 0; i < HEIGHT; i++) {
    for (var j = 0; j < WIDTH; j++) {
      board[i][j] = "";
      $("#h" + i + "w" + j).text("");
      $("#h" + i + "w" + j).removeClass("blinking");
    }
  }
  if (token == playerToken) {
    $('#playerwin').modal({ dismissible: false});
    $('#playerwin').modal("open");
  } else {
    $('#compwin').modal({ dismissible: false});
    $('#compwin').modal("open");
  }
}

function draw() {
  for (var i = 0; i < HEIGHT; i++) {
    for (var j = 0; j < WIDTH; j++) {
      board[i][j] = "";
      $("#h" + i + "w" + j).text("");
      $("#h" + i + "w" + j).removeClass("blinking");
    }
  }
  $('#draw').modal({ dismissible: false});
  $("#draw").modal("open");
}

function playerTurn() {
  $("#turn").text("It is your turn! click on a square to place a token into that column.");
  $("#turn").css("color", "blue");
  $(".cell").on("click", function (e) {
    skipCounter = 0;
    if (!isPlayerTurn) return; // dont do anything
    console.log("Player turn");
    if ($(e.target).text().length === 0) {
      var id = $(e.target).attr("id");
      var col = parseInt(id[3]);
      // find lowest point to place the token in
      for (var i = HEIGHT-1; i >= 0; i--) {
        if (board[i][col] == '') {
          board[i][col] = playerToken;
          $("#h" + i + "w" + col).text(playerToken);
          break;
        }
      }
      $(".cell").off();
      if (!checkWin()) {
        isPlayerTurn = false;
        setTimeout(function () { runAI(board) }, 250);
      }
    }
  });
}

$(document).ready(function () {
  $('#modal1').modal({ dismissible: false});
  $('#modal1').modal('open');

  $(".X").on("click", function () {
    $('#modal1').modal('close');
    compToken = "O";
    playerToken = "X";
    runAI(board);
  });

  $(".O").on("click", function () {
    $('#modal1').modal('close');
    compToken = "X";
    playerToken = "O";
    isPlayerTurn = true;
    playerTurn();
  });
  // in this game we can skip
  $("#skip").on("click", function() {
    skipCounter++;
    if (isPlayerTurn) {
      isPlayerTurn = false;
      runAI(board);
    }
  });
});