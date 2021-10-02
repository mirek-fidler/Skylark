(function () {
    var r = new Resumable({
        target: '/upload',
        query: {},
        maxChunkRetries: 2,
        maxFiles: 1,
        testChunks : false,
        prioritizeFirstAndLastChunk: false,
        simultaneousUploads: 1,
        chunkSize: 1 * 1024 * 1024
    });
    var results = $('#results'),
        draggable = $('#dragHere'),
        uploadFile = $('#uploadFiles'),
        browseButton = $('#browseButton'),
        nothingToUpload = $('[data-nothingToUpload]');
        


    // if resumable is not supported aka IE
    if (!r.support) location.href = 'http://browsehappy.com/';

    r.assignBrowse(browseButton);
    r.assignDrop(draggable);

    r.on('fileAdded', function (file, event) {
        var template =
            '<div data-uniqueid="' + file.uniqueIdentifier + '">' +
            '<div class="fileName">' + file.fileName + ' (' + file.file.type + ')' + '</div>' +
            '<div class="large-6 right deleteFile">X</div>' +
            '<div class="progress large-6">' +
            '<span class="meter" style="width:0%;"></span>' +
            '</div>' +
            '</div>';

        results.append(template);
    });

    uploadFile.on('click', function () {
        if (results.children().length > 0) {
            r.upload();
        } else {
            nothingToUpload.fadeIn();
            setTimeout(function () {
                nothingToUpload.fadeOut();
            }, 3000);
        }
    });
    
     $(document).on('click', '.refreshDownload', function () {
        console.log("refresh");
        location.reload();
    });

    $(document).on('click', '.deleteFile', function () {
        var self = $(this),
            parent = self.parent(),
            identifier = parent.data('uniqueid'),
            file = r.getFromUniqueIdentifier(identifier);

        r.removeFile(file);
        parent.remove();
    });




    r.on('fileProgress', function (file) {
        var progress = Math.floor(file.progress() * 100);
        $('[data-uniqueId=' + file.uniqueIdentifier + ']').find('.meter').css('width', progress + '%');
        $('[data-uniqueId=' + file.uniqueIdentifier + ']').find('.meter').html('&nbsp;' + progress + '%');
    });

    r.on('fileSuccess', function (file, message) {
        $('[data-uniqueId=' + file.uniqueIdentifier + ']').find('.progress').addClass('success');
    });


    r.on('uploadStart', function () {
        $('.alert-box').text('Uploading....');
    });

    r.on('complete', function () {
        $('.alert-box').text('Done Uploading');
    });

})();

//TotalD
//FileID

$(function(){
    $('.generateURL').popover({
        placement: 'top',
        sanitize: false,
        html:true,
        content:  function(){ $('#FormGenerateURL').find(".FileID").attr('value',$(this).attr('id'));   return $('#FormGenerateURL').html();}
    });
})

// 